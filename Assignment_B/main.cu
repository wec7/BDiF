/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: CVA Main file
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

// thrust includes

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/transform.h>
#include <thrust/transform_scan.h>
#include <thrust/reduce.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/zip_iterator.h>
#include <thrust/iterator/transform_iterator.h>
#include <thrust/iterator/discard_iterator.h>
#include <thrust/execution_policy.h>

// boost includes

#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

// cuda includes

#include <nvml.h>
#include <omp.h>

// C++ includes

#include <unistd.h>
#include <fstream>

// local includes

#include "counterparty.h"
#include "gpu_helper.h"
#include "logFile.h"
#include "output.h"
#include "params.h"
#include "swap.h"
#include "timer.h"

int main(int argc, char **argv) {

    // read the simulation & model parameters and store them in data structures
    
    std::ifstream params_file(argv[1]);
    Params in = Params(params_file); 
    params_file.close();

    // open log file and write initial output

    Output::file() = fopen(in.log_file.c_str(), "a");
    LogFile::ReportLevel() = stringToLevel(in.log_level);
    LOG(OFF) << "EXECUTE\t" << argv[0];
    LOG(INFO) << "CONFIG\t" << argv[1];

    // Divide to CPU and GPU, write openmp info

#ifdef _CPU_
    LOG(OFF) << "CUDA SYSTEM\tOpenMP (CPU)";
    LOG(INFO) << "NUM_PROCESSORS\t" << omp_get_num_procs();
    LOG(INFO) << "NUM_THREADS\t" << omp_get_max_threads();
#endif

#ifdef _GPU_
    LOG(OFF) << "CUDA SYSTEM\tCUDA (GPU)";
    int num_gpus;
    cudaGetDeviceCount(&num_gpus);
    LOG(INFO) << "NUM_GPUs\t" << num_gpus;
    for (unsigned i = 0; i < num_gpus; ++i) {
        cudaDeviceProp dprop;
        cudaGetDeviceProperties(&dprop, i);
        LOG(INFO) << "   GPU " << i << " : " << dprop.name;
    }
    nvmlInit();
    char version[NVML_SYSTEM_NVML_VERSION_BUFFER_SIZE];
    if (nvmlSystemGetDriverVersion(version, NVML_SYSTEM_NVML_VERSION_BUFFER_SIZE) == NVML_SUCCESS) {
        LOG(INFO) << "chenggong : " << version;
    }
#endif

// TODO: how to get CUDA version, Thrust version, GPU memory usage info
// one possible way may be using system()
// cat /proc/cpuinfo | grep "model name" | awk -F: '{print $2}'
//=========================================================
    std::vector<float> hazard_rate(in.num_counterparties);
    std::vector<float> fx_net_nominal(in.num_counterparties, 0.0f);
    std::vector<float> usd_fixed(in.num_counterparties * in.tenor[1]*12, 0.0f);
    std::vector<float> eur_fixed(in.num_counterparties * in.tenor[1]*12, 0.0f);
    std::vector<float> usd_floating(in.num_counterparties * in.tenor[1]*12, 0.0f);
    std::vector<float> eur_floating(in.num_counterparties * in.tenor[1]*12, 0.0f);

    // Counterparties, deals and netting sets simulation
    LOG(INFO) << "Simulating Bank Data=================";
    // Generate counterparties
    LOG(INFO) << "Generating " << in.num_counterparties << " Counterparties...";
    std::vector<unsigned> idx[5]; // these vectors help us to find where is the ith A rating counterparty
    std::vector<Counterparty> counterparties = create_counterparties(in, hazard_rate, idx);
    // Generate forex deals
    LOG(INFO) << "Generating " << in.num_fx << " Forex EUR/USD Deals...";
    std::vector<Swap> fx_deals = create_fxs(in);
    // Generate swap deals
    LOG(INFO) << "Generating " << in.num_swap << " Swap Deals...";
    std::vector<Swap> swap_deals = create_swaps(in);
    // Netting sets
    LOG(INFO) << "Allocating Deals to Counterparties...";
    allocate(counterparties, fx_deals, swap_deals, in, idx, fx_net_nominal, usd_fixed, eur_fixed, usd_floating, eur_floating);

    std::ofstream fout("output.data");
    boost::archive::binary_oarchive oar(fout);
    oar << hazard_rate << fx_net_nominal << usd_fixed << eur_fixed << usd_floating << eur_floating;

    if (LogFile::ReportLevel() >= DIAG) {
        LOG(DIAG) << "Counterparty 0 EUR Swap : Net Fixed-leg Cash Flow at each time step (month)"; // Only log the first counterparty EUR Swap
        for (unsigned i = 0; i < in.tenor[1]*12; ++i)
        { LOG(DIAG) << "     " << i + 1 << " : " << eur_fixed[i]; }
        for (unsigned i = 0; i < in.num_counterparties; ++i)
        { LOG(DIAG) << "Forex Net Nominal of Counterparty " << i << " : " << fx_net_nominal[i]; }
    }


//===================================================================================
// Combine OpenMP and CUDA Thrust to do multi-GPU Monte Carlo
//===================================================================================
  Timer timer; // Set up timer
  timer.start();
  LOG(INFO) << "Monte Carlo Parameters===============";
  LOG(INFO) << "Number of Paths : " << in.num_paths;
  LOG(INFO) << "Number of Years : " << in.tenor[1];
  LOG(INFO) << "Number of Steps per Year : " << 12.;
  LOG(INFO) << "Number of Steps along a Path : " << in.tenor[1]*12;
  float dt = 1.0f / 12.;
  float mean_fx = (in.fx_drift - in.fx_sigma * in.fx_sigma / 2.0f) * dt; // (r - sigma^2 / 2) * dt
  float stdev_fx = in.fx_sigma * sqrt(dt); // sigma * sqrt(dt)

#ifdef _GPU_ // Monte Carlo on multi-GPU
  LOG(INFO) << "Monte Carlo on Multi-GPU=============";
  thrust::host_vector<float> hCVA[num_gpus]; // final CVA numbers for each counterparty end up in hCVA[0]
  for (unsigned j = 0; j < num_gpus; ++j) { hCVA[j].resize(in.num_counterparties); }
  omp_set_num_threads(num_gpus);
  #pragma omp parallel
  {
    using namespace thrust;
    Timer loctimer; // Set up local timer
    loctimer.start();
    int i = omp_get_thread_num();
    cudaSetDevice(i);

    // Copy data from host
    int size0 = in.tenor[1]*12, size1 = in.num_counterparties, size2 = in.tenor[1]*12 * in.num_counterparties;
    device_vector<float> dFX_Price(size0); // Future price of Forex EUR/USD from time 0 to in.tenor[1] - dt
    device_vector<float> dUSD_Curve[4], dEUR_Curve[4]; // Curve parameters from time 0 to in.tenor[1] - dt
    for (unsigned j = 0; j < 4; ++j) { dUSD_Curve[j].resize(size0); dEUR_Curve[j].resize(size0); }
    device_vector<float> dHazard_rate(size1), dFX_nominal(size1), dSwap(size1), dCVA(size1, 0.0f);
    device_vector<float> dUSD_fixed(size2), dEUR_fixed(size2), dUSD_floating(size2), dEUR_floating(size2), dIdx(size2);
    counting_iterator<unsigned> beg(0), end0(size0), end2(size2);
    thrust::transform(beg, end2, dIdx.begin(), getIdx(in.tenor[1]*12));
    thrust::copy(fx_net_nominal.begin(), fx_net_nominal.end(), dFX_nominal.begin());
    thrust::copy(hazard_rate.begin(), hazard_rate.end(), dHazard_rate.begin());
    thrust::copy(usd_fixed.begin(), usd_fixed.end(), dUSD_fixed.begin());
    thrust::copy(eur_fixed.begin(), eur_fixed.end(), dEUR_fixed.begin());
    thrust::copy(usd_floating.begin(), usd_floating.end(), dUSD_floating.begin());
    thrust::copy(eur_floating.begin(), eur_floating.end(), dEUR_floating.begin());
    typedef thrust::device_vector<float>::iterator FloatIterator;
    typedef thrust::tuple<FloatIterator, FloatIterator, FloatIterator, FloatIterator> TupleFloatIterator;
    typedef thrust::zip_iterator<TupleFloatIterator> ZipIterator;
    ZipIterator first = make_zip_iterator(make_tuple(dSwap.begin(), dFX_nominal.begin(), dHazard_rate.begin(), dCVA.begin()));
    ZipIterator last = make_zip_iterator(make_tuple(dSwap.end(), dFX_nominal.end(), dHazard_rate.end(), dCVA.end()));

    // Figure out the number of paths this gpu is going to deal with
    int num_paths = (i == 0) ? in.num_paths - in.num_paths / num_gpus * (num_gpus - 1) : in.num_paths / num_gpus;
    LOG(INFO) << "   GPU " << i << " : " << num_paths << " paths";

    for (unsigned p = 0; p < num_paths; ++p) {
      // Simulate a path
      // We need 1000000i to avoid the same seed in different GPUs
      // We need 10p to make sure seeds are different in different paths (9 rng constructors per path)
      int seed = 1000000 * i + 10 * p;
      transform_exclusive_scan(beg, end0, dFX_Price.begin(), random_lognormal(mean_fx, stdev_fx, seed), in.fx_init, multiplies<float>());
      // Curve parameters may go negative since we use OU process but not CIR model
      for (unsigned j = 0; j < 4; ++j) {
        transform_exclusive_scan(beg, end0, dUSD_Curve[j].begin(), getA(in.usd_params[j][2], dt, seed+2*j+1), 0, plus<float>());
        transform_exclusive_scan(beg, end0, dEUR_Curve[j].begin(), getA(in.eur_params[j][2], dt, seed+2*j+2), 0, plus<float>());
        thrust::transform(make_zip_iterator(make_tuple(dUSD_Curve[j].begin(), beg)),
            make_zip_iterator(make_tuple(dUSD_Curve[j].end(), end0)), dUSD_Curve[j].begin(), getCurve(in.usd_params[j], dt));
        thrust::transform(make_zip_iterator(make_tuple(dEUR_Curve[j].begin(), beg)),
            make_zip_iterator(make_tuple(dEUR_Curve[j].end(), end0)), dEUR_Curve[j].begin(), getCurve(in.eur_params[j], dt));
      }
      if (LogFile::ReportLevel() >= DIAG) {
          if ((i == 0) && (p == 0)) {
              for (int j = 0; j < in.tenor[1]*12; ++j)
              { LOG(DIAG) << "   GPU " << i << " : USD Curve Beta1 (Step " << j << ") = " << dUSD_Curve[1][j]; }
          }
      }
      
      // For each counterparty, accumulate CVA along the path
      for (unsigned j = 0; j < in.tenor[1]*12; ++j) {
        priceSwap pricer(dUSD_Curve[0][j], dUSD_Curve[1][j], dUSD_Curve[2][j], dUSD_Curve[3][j],
                         dEUR_Curve[0][j], dEUR_Curve[1][j], dEUR_Curve[2][j], dEUR_Curve[3][j], in.tenor[1]*12, j, dt, dFX_Price[j]);
        reduce_by_key(dIdx.begin(), dIdx.end(), make_transform_iterator(make_zip_iterator(make_tuple(beg, dUSD_fixed.begin(), dEUR_fixed.begin(),
                      dUSD_floating.begin(), dUSD_floating.begin())), pricer), make_discard_iterator(), dSwap.begin());
        thrust::transform(first, last, dCVA.begin(), accumulateCVA(j, dFX_Price[j], in.disc_rate, dt));
      }
    } // path loop

    thrust::copy(dCVA.begin(), dCVA.end(), hCVA[i].begin());
    loctimer.end();
    LOG(INFO) << "Time (GPU " << i << ") : " << std::setprecision(3) << std::fixed << loctimer.d_time << " s";
  } // omp parallel

  for (unsigned j = 1; j < num_gpus; ++j) {
      thrust::transform(thrust::make_zip_iterator(thrust::make_tuple(hCVA[0].begin(), hCVA[j].begin())),
          thrust::make_zip_iterator(thrust::make_tuple(hCVA[0].end(), hCVA[j].end())), hCVA[0].begin(), add());
  }
  thrust::for_each(hCVA[0].begin(), hCVA[0].end(), divide(in.num_paths));
  float aggregateCVA = thrust::reduce(hCVA[0].begin(), hCVA[0].end());
  LOG(OFF) << "Aggregate CVA for the bank : " << aggregateCVA;
  if (LogFile::ReportLevel() >= DIAG) {
      for (int j = 0; j < in.num_counterparties; ++j)
      { LOG(DIAG) << "CVA for Counterparty " << j << " (hazard rate " << hazard_rate[j] << ") : " << hCVA[0][j]; }
  }

#endif

#ifdef _CPU_
    // Prepare data
    LOG(INFO) << "Preparing Data=======================";
    thrust::host_vector<float> CVA(in.num_counterparties, 0.0f);  // the final CVA numbers end up in this vector
    typedef thrust::device_vector<float>::iterator FloatIterator;
    typedef thrust::tuple<FloatIterator, FloatIterator, FloatIterator> TupleFloatIterator3;
    typedef thrust::zip_iterator<TupleFloatIterator3> ZipIterator;
    LOG(INFO) << "Initializing Data for CPUs===========";
    thrust::device_vector<float> dFX_net_nominal = fx_net_nominal;
    thrust::device_vector<float> dFX_price(in.tenor[1]*12);
    thrust::device_vector<float> dCVA(in.num_counterparties, 0.0f);
    thrust::device_vector<float> dHazard_rate = hazard_rate;
    // Put the net nominal, hazard rate and CVA together
    ZipIterator first = thrust::make_zip_iterator(thrust::make_tuple(dFX_net_nominal.begin(), dHazard_rate.begin(), dCVA.begin()));
    ZipIterator last  = thrust::make_zip_iterator(thrust::make_tuple(dFX_net_nominal.end(), dHazard_rate.end(), dCVA.begin()));
    for (unsigned p = 0; p < in.num_paths; ++p) {
        // Simulate a path
        // It's possible that two iterations are in the same second. That's why we need +p in the constructor of random_normal.
        thrust::transform_inclusive_scan(thrust::counting_iterator<unsigned>(0), thrust::counting_iterator<unsigned>(in.tenor[1]*12),
                                         dFX_price.begin(), random_normal(mean_fx, stdev_fx, p), thrust::plus<float>());
        thrust::transform(dFX_price.begin(), dFX_price.end(), dFX_price.begin(), generatePrice(in.fx_init));
        // For each counterparty, accumulate CVA along the path
        for (unsigned j = 0; j < in.tenor[1]*12; ++j) {
            thrust::transform(first, last, dCVA.begin(), incrementCVA(in.disc_rate, dFX_price[j], (j + 1) * dt, dt));
        }
        LOG(DIAG) << "Path " << p + 1 << " Finishs...";
    }
    thrust::copy(thrust::make_transform_iterator(dCVA.begin(), normalizeCVA(in.num_paths)),
                 thrust::make_transform_iterator(dCVA.end(), normalizeCVA(in.num_paths)), CVA.begin());
    // Calculate aggregate CVA
    float aggregateCVA = thrust::reduce(CVA.begin(), CVA.end());
    LOG(OFF) << "Aggregate CVA for the bank : " << aggregateCVA;
#endif

    timer.end();
    LOG(OFF) << "Time for Path Simulation and CVA Calculation : " << std::setprecision(3) << std::fixed << timer.d_time << " s";
    LOG(OFF) << argv[0] << " Ends Here...====================================";
    return 0;
}
