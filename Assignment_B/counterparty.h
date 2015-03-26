/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Counterparty class
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef COUNTERPARTY_H
#define COUNTERPARTY_H

// C++ includes
#include <vector>

// local includes
#include "params.h"
#include "swap.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>

class Counterparty {
public:
    char rating;
    float hazard_rate;
    Counterparty(int rating_num, float hazard_rate); 
};

std::vector<Counterparty> create_counterparties(Params& in, std::vector<float>& hazard_rate, std::vector<unsigned> *idx);
void allocate(std::vector<Counterparty> & cps, std::vector<Swap> & fx, std::vector<Swap> & swap,
              Params & in, std::vector<unsigned> * idx,
              std::vector<float> & fx_net_nominal,
              std::vector<float> & usd_cash_flows, std::vector<float> & eur_cash_flows,
              std::vector<float> & usd_floating, std::vector<float> & eur_floating);

#endif