/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Params Class implementation, read the simulation & model parameters and store them in data structures 
(for example a C/C++ structs).
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

// local include
#include "params.h"

// C++ includes
#include <vector>
#include <cstdlib>
#include <cmath>
#include <fstream>

// boost includes
#include <boost/algorithm/string.hpp>

Params::Params(std::ifstream& file) {
    /*
    @summary: Params Class constructor implementation
    @param file: input file including paramter infomation
    */
    std::string str_line;
    while (getline(file, str_line)) {
        std::vector<std::string> vec_keys;
        boost::split(vec_keys, str_line, boost::is_any_of(","));

        // log paramters
        if (vec_keys[0] == "log_file") log_file = vec_keys[1];
        if (vec_keys[0] == "log_level") log_level = vec_keys[1];

        // counterparty paramters
        if (vec_keys[0] == "num_counterparties") 
            num_counterparties = atoi(vec_keys[1].c_str());
        if (vec_keys[0] == "ratings") 
            for (int i = 0; i < 5; ++i)
                ratings[i] = atof(vec_keys[i+1].c_str());
        if (vec_keys[0] == "hazard")
            for (int i = 0; i < 5; ++i)
                hazard[i] = atof(vec_keys[i+1].c_str());
        if (vec_keys[0] == "allocate")
            for (int i = 0; i < 5; ++i)
                allocate[i] = atof(vec_keys[i+1].c_str());

        // deals paramters
        if (vec_keys[0] == "num_fx") num_fx = atoi(vec_keys[1].c_str());
        if (vec_keys[0] == "num_swap") num_swap = atoi(vec_keys[1].c_str());
        if (vec_keys[0] == "fx_long_percent") fx_long_percent = atof(vec_keys[1].c_str());
        if (vec_keys[0] == "swap_long_percent") swap_long_percent = atof(vec_keys[1].c_str());
        if (vec_keys[0] == "fx_nominal") {
            fx_nominal[0] = atof(vec_keys[1].c_str());
            fx_nominal[1] = atof(vec_keys[2].c_str());
        } 
        if (vec_keys[0] == "swap_nominal") {
            swap_nominal[0] = atof(vec_keys[1].c_str());
            swap_nominal[1] = atof(vec_keys[2].c_str());
        } 
        if (vec_keys[0] == "fixed_rate") {
            fixed_rate[0] = atof(vec_keys[1].c_str());
            fixed_rate[1] = atof(vec_keys[2].c_str());
        } 
        if (vec_keys[0] == "tenor") {
            tenor[0] = atoi(vec_keys[1].c_str());
            tenor[1] = atoi(vec_keys[2].c_str());
        } 

        // monte carlo paramters
        if (vec_keys[0] == "num_paths") num_paths = atoi(vec_keys[1].c_str());
        if (vec_keys[0] == "disc_rate") disc_rate = atof(vec_keys[1].c_str());
        if (vec_keys[0] == "fx_init") fx_init = atof(vec_keys[1].c_str());
        if (vec_keys[0] == "fx_drift") fx_drift = atof(vec_keys[1].c_str());
        if (vec_keys[0] == "fx_sigma") fx_sigma = atof(vec_keys[1].c_str());

        // other paramters for USD and EUR
        if (vec_keys[0] == "usd_beta0")
            for (int i = 0; i < 4; ++i) 
                usd_params[0][i] = atof(vec_keys[i+1].c_str());
        if (vec_keys[0] == "usd_beta1")
            for (int i = 0; i < 4; ++i)
                usd_params[1][i] = atof(vec_keys[i+1].c_str());
        if (vec_keys[0] == "usd_beta2")
            for (int i = 0; i < 4; ++i)
                usd_params[2][i] = atof(vec_keys[i+1].c_str());
        if (vec_keys[0] == "usd_beta3")
            for (int i = 0; i < 4; ++i) 
                usd_params[3][i] = atof(vec_keys[i+1].c_str());
        if (vec_keys[0] == "eur_beta0") 
            for (int i = 0; i < 4; ++i) 
                eur_params[0][i] = atof(vec_keys[i+1].c_str());
        if (vec_keys[0] == "eur_beta1")
            for (int i = 0; i < 4; ++i) 
                eur_params[1][i] = atof(vec_keys[i+1].c_str());
        if (vec_keys[0] == "eur_beta2")
            for (int i = 0; i < 4; ++i) 
                eur_params[2][i] = atof(vec_keys[i+1].c_str());
        if (vec_keys[0] == "eur_beta3")
            for (int i = 0; i < 4; ++i) 
                eur_params[3][i] = atof(vec_keys[i+1].c_str());
    }
}
