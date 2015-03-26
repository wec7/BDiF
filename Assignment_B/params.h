/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Params Class, read the simulation & model parameters and store them in data structures 
(for example a C/C++ structs).
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef PARAMS_H
#define PARAMS_H

// C++ includes
#include <string>

class Params{
    /* summary: stored class of simulation & model parameters */
public:
    /* 
    @summary: Log file parameters
    @mem log_file: log output file name
    @mem log_level: INFO, DIAG or CALL
    */
    std::string log_file, log_level;

    /*
    @summary: Counterparty parameters
    @mem num_counterparties: number of counterparties, default 50000
    @mem rating: rating 1.0 default
    @mem hazard: hazard rate to decide default, default value 0.02, 0.04 ...
    @mem allocate: allocation paramters, default value 2^i
    */
    unsigned num_counterparties;
    float ratings[5], hazard[5], allocate[5];
    
    /*
    @summary: Deals parameters
    @mem num_fx, num_swap: number of FX and number of swaps
    @mem fx_nominal, swap_nominal: 800000, 12000000 by default
    @mem tenor: Tenor of swap, 2 months or 50 months by default
    @mem fixed_rate: fixed rate of swap, 0.02 and 0.08 by default
    */
    unsigned long num_fx, num_swap;
    float fx_long_percent, swap_long_percent;
    float fx_nominal[2], swap_nominal[2];
    unsigned tenor[2];
    float fixed_rate[2];
    
    /*
    @summary: Monte Carlo parameters
    @mem num_paths: number of paths for monte carlo simulation, 50000 by default
    @mem disc_rate: discount rate, 0.6 by default
    @mem fx_init, fx_drift, fx_sigma: FX parameters
    @mem usd_params, eur_params: initial value, x-bar, mean-reversion(alpha), volatility
    */
    unsigned num_paths;
    float disc_rate;
    float fx_init, fx_drift, fx_sigma;
    float usd_params[4][4], eur_params[4][4];

    /* constructor */
    Params(std::ifstream& file); 
};

#endif
