#ifndef FUNCTORS_H
#define FUNCTORS_H

#include <cmath>
#include <ctime>

#include <thrust/random/linear_congruential_engine.h>
#include <thrust/random/normal_distribution.h>

struct random_lognormal : public thrust::unary_function<int, float> {
    private:
        thrust::minstd_rand rng;
        thrust::random::normal_distribution<float> normal_dist;
    public:
        __device__ __host__
        inline random_lognormal(float mean = 0, float stdev = 1, int seed = 0): normal_dist(mean, stdev), rng(time(NULL) + seed) {}
	      __device__ __host__
	      inline float operator()(int index) {
            // discard the random numbers used before
            // this is important since otherwise we will get exactly the same number again and again
		        rng.discard(index);
		        return exp(normal_dist(rng));
        }
};

// A is the ito integral in the expression of curve parameters
struct getA : public thrust::unary_function<int, float> {
    private:
        thrust::minstd_rand rng;
        thrust::random::normal_distribution<float> normal_dist;
        float a, b; // a = exp(alpha * dt), b = sqrt(dt)
    public:
        __device__ __host__
        inline getA(float alpha, float dt, int seed) : a(exp(alpha * dt)), b(sqrt(dt)), rng(time(NULL) + seed) {}
	      __device__ __host__
	      inline float operator()(int i) {
            // discard the random numbers used before
            // this is important since otherwise we will get exactly the same number again and again
		        rng.discard(i);
		        return pow(a, i) * b * normal_dist(rng); // effectively exp(alpha * dt * i) * sqrt(dt) * Z
        }
};

struct getCurve : public thrust::unary_function<thrust::tuple<float, int>, float> {
    private:
        float x0, xbar, alpha, sigma, dt;
    public:
        __device__ __host__
        inline getCurve(float * params, float _dt) : x0(params[0]), xbar(params[1]), alpha(params[2]), sigma(params[3]), dt(_dt) {}
        __device__ __host__
        inline float operator()(const thrust::tuple<float, int> & tuple) {
            return xbar + (x0 - xbar + sigma * thrust::get<0>(tuple)) * exp (- alpha * dt * thrust::get<1>(tuple));
            // effectively x0 * exp(- alpha * dt * i) + xbar * (1 - exp(- alpha * dt * i)) + exp(- alpha * dt * i) * sigma * A
        }
};

struct getIdx : public thrust::unary_function<int, int> {
    private:
        int num_steps;
    public:
        __device__ __host__
        inline getIdx(int _num_steps) : num_steps(_num_steps) {}
        __device__ __host__
        inline int operator()(int i) {
            return i / num_steps;
        }
};

struct accumulateCVA : public thrust::unary_function<thrust::tuple<float, float, float, float>, float> {
    //swap value, fx net nominal, hazard rate, cva
    private:
        float fx_rate, discount_rate, ref_step, dt;
    public:
        __device__ __host__
        inline accumulateCVA(int _ref_step, float _fx_rate, float _discount_rate, float _dt) 
        : discount_rate(_discount_rate), fx_rate(_fx_rate), ref_step(_ref_step), dt(_dt) {}
        __device__ __host__
        inline float operator()(const thrust::tuple<float, float, float, float> & tp) {
            float netExpo = thrust::get<0>(tp) + thrust::get<1>(tp) * fx_rate; // net exposure
            if (netExpo <= 0) { return thrust::get<3>(tp); }
            float hr = thrust::get<2>(tp); // hazard_rate
            float prob = exp(- hr * dt * ref_step) - exp(- hr * dt * (ref_step + 1));
            return thrust::get<3>(tp) + netExpo * prob * exp(- discount_rate * dt * ref_step);
        }
};

struct add : public thrust::unary_function<thrust::tuple<float, float>, float> {
    public:
        __device__ __host__
        inline float operator()(const thrust::tuple<float, float> & tp) {
            return thrust::get<0>(tp) + thrust::get<1>(tp);
        }
};

struct divide : public thrust::unary_function<float, float> {
    private:
        float divisor;
    public:
        __device__ __host__
        inline divide(float denominator) : divisor(denominator) {}
        __device__ __host__
        inline int operator()(float f) {
            return f / divisor;
        }
};


struct priceSwap : public thrust::unary_function<thrust::tuple<int, float, float, float, float>, float> {
    // step, usd_fixed, usd_fixed, usd_floating, eur_floating
    private:
        int num_steps, ref_step;
        float dt, fx_rate;
        float USD_b0, USD_b1, USD_b2, USD_l;
        float EUR_b0, EUR_b1, EUR_b2, EUR_l;
    private:
        __device__ __host__
        inline float discount_factor(float t, float b0, float b1, float b2, float l) {
            float expo = exp(- t / l);
            return exp(- (b0 + (b1 + b2) * l / t * (1 - expo) - b2 * expo) * t);
        }
        __device__ __host__
        inline float USD_df(float t) { return discount_factor(t, USD_b0, USD_b1, USD_b2, USD_l); }
        __device__ __host__
        inline float EUR_df(float t) { return discount_factor(t, EUR_b0, EUR_b1, EUR_b2, EUR_l); }
    public:
        __device__ __host__
        inline priceSwap(float usd_b0, float usd_b1, float usd_b2, float usd_l,
                          float eur_b0, float eur_b1, float eur_b2, float eur_l,
                          int _num_steps, int _ref_step, float _dt, float _fx_rate)
        : USD_b0(usd_b0), USD_b1(usd_b1), USD_b2(usd_b2), USD_l(usd_l),
          EUR_b0(eur_b0), EUR_b1(eur_b1), EUR_b2(eur_b2), EUR_l(eur_l),
          num_steps(_num_steps), ref_step(_ref_step), dt(_dt), fx_rate(_fx_rate) {}
        __device__ __host__
        inline float operator()(const thrust::tuple<int, float, float, float, float> & tp) {
            int k = thrust::get<0>(tp) % num_steps - ref_step;
            float pv = 0;
            if (k == 0) { pv = thrust::get<3>(tp) + fx_rate * thrust::get<4>(tp); }
            if (k >= 0) { pv += thrust::get<1>(tp) * USD_df((k + 1) * dt) + fx_rate * thrust::get<2>(tp) * EUR_df((k + 1) * dt); }
            return pv;
        }
};

#endif
