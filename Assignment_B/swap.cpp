/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Swap Class implementation
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

// C++ includes
#include <cstdlib>
#include <ctime>

// local includes
#include "swap.h"
using namespace std;

vector<Swap> create_swaps(Params & in) {
    /* 
    @summary: helper function to create a vector of swaps
    @param in: input class, includes related parameters to initialize swaps
    @return: a vector of swaps
    */

    // initialization
    vector<Swap> swaps;
    float nominal_b = in.swap_nominal[0], nominal_k = (in.swap_nominal[1] - nominal_b) / RAND_MAX;
    float fixed_b = in.fixed_rate[0], fixed_k = (in.fixed_rate[1] - fixed_b) / RAND_MAX;
    int tenor_b = in.tenor[0] * 12., tenor_k = (in.tenor[1] -in.tenor[0]) * 12. + 1;

    // creation
    for (size_t i = 0; i < in.num_swap; ++i) {
        int bool_long = 1 - 2 * int(static_cast<float>(rand())/RAND_MAX < in.swap_long_percent);
        string ccy = (static_cast<float>(rand())/RAND_MAX < 0.5) ? "EUR" : "USD";
        swaps.push_back(Swap(ccy, 
            (nominal_b + nominal_k * rand()) * bool_long, 
            fixed_b + fixed_k * rand(), 
            tenor_b + rand() % tenor_k));
    }
    return swaps;
}


std::vector<Swap> create_fxs(Params & in) {
    /*
    @summary: helper function to create a vector FX options
    @param in: input class, inclues related parameters to initialize FXs
    @mem bool_long: long - positive nominal, short - negative nominal
    @return a vector FXs
    */

    // initialization
    vector<Swap> fxs;
    float nominal_b = in.fx_nominal[0], nominal_k = (in.fx_nominal[1] - nominal_b) / RAND_MAX;

    // creation
    for (size_t i = 0; i < in.num_fx; ++i) {
        int bool_long = 1 - 2*int(static_cast<float>(rand()) / RAND_MAX < in.fx_long_percent);
        fxs.push_back(Swap((nominal_b + nominal_k * rand()) * bool_long));
    }
    return fxs;
}
