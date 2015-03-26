/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Counterparty class implementation
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
Reference: allocate function referenced from Yu Gan
*/

#include <ctime>
#include <cstdlib>
#include "Counterparty.h"

Counterparty::Counterparty(int rating_num, float hazard_rate) {
    /* 
    @summary: Counterparty class constructor
    @param rating_num: 0(A), 1(B), 2(C), 3(D), 4(E)
    @param harzard_rate: harzard rate
    */
    rating = 'A' + rating_num;
    hazard_rate = hazard_rate;
}

std::vector<Counterparty> create_counterparties(Params &in, std::vector<float> &hazard_rate, std::vector<unsigned> *idx) {
    /*
    @summary: create a vector of counterparites
    @param in: input parameters contain related values
    @param hazard rate: hazard rates 
    @return: a vector of counterparities
    */
    std::vector<Counterparty> cps;
    float t = in.ratings[0] + in.ratings[1] + in.ratings[2] + in.ratings[3] + in.ratings[4];
    float a = in.ratings[0] / t;
    float b = a + in.ratings[1] / t;
    float c = b + in.ratings[2] / t;
    float d = c + in.ratings[3] / t;
    float r;
    for (size_t i = 0; i < in.num_counterparties; ++i) {
        r = static_cast<float>(rand()) / RAND_MAX;
        if (r < a) { 
            cps.push_back(Counterparty(0, in.hazard[0]));
            hazard_rate[i] = in.hazard[0]; 
            idx[0].push_back(i); 
        }
        else if (r < b) { 
            cps.push_back(Counterparty(1, in.hazard[1]));
            hazard_rate[i] = in.hazard[1]; 
            idx[1].push_back(i); 
        }
        else if (r < c) { 
            cps.push_back(Counterparty(2, in.hazard[2]));
            hazard_rate[i] = in.hazard[2]; 
            idx[2].push_back(i); 
        }
        else if (r < d) { 
            cps.push_back(Counterparty(3, in.hazard[3])); 
            hazard_rate[i] = in.hazard[3]; 
            idx[3].push_back(i); 
        }
        cps.push_back(Counterparty(4, in.hazard[4]));
        hazard_rate[i] = in.hazard[4]; 
        idx[4].push_back(i);
    }
    return cps;
}

class add{
public:
    float increment;
    add(float incre) : increment(incre) {}
    void operator()(float &f) { f += increment; }
};

class Rating{
public:
    float perc[4];
    Rating(Params & in) {
        float total = in.allocate[0] + in.allocate[1] + in.allocate[2] + in.allocate[3] + in.allocate[4];
        perc[0] = in.allocate[0] / total;
        perc[1] = in.allocate[1] / total + perc[0];
        perc[2] = in.allocate[2] / total + perc[1];
        perc[3] = in.allocate[3] / total + perc[2];
    }
    int operator()() {
        float r = static_cast<float>(rand()) / RAND_MAX;
        if (r > perc[3]) return 4;
        if (r > perc[2]) return 3;
        if (r > perc[1]) return 2;
        if (r > perc[0]) return 1;
        return 0;
    }
};

void allocate(std::vector<Counterparty> & cps, std::vector<Swap> & fx, std::vector<Swap> & swap,
              Params & in,  std::vector<unsigned> * idx,
              std::vector<float> & fx_net_nominal,
              std::vector<float> & usd_cash_flows, std::vector<float> & eur_cash_flows,
              std::vector<float> & usd_floating, std::vector<float> & eur_floating) {
    /*
    @summary: allocate deals to counterpary, swap and fx
    @param cps: counterparties
    @param fx: forex
    @param swap: swaps
    @param in: paramter files
    @param idx: from counterparties creation
    @param fx_net_nominal: forex net nominals
    @param usd_cash_flows, eur_cash_flows: cash flows
    @param usd_floating, eur_floating: floating rates
    @return: none, make change to cps, fx and swap
    */

    // counterparty

    for (unsigned i = 0; i < in.num_counterparties; ++i)
        fx_net_nominal[i] = fx[i].nominal;
    srand(time(NULL));
    Rating rand_rating(in);

    // FX

    for (unsigned long i = in.num_counterparties; i < in.num_fx; ++i) {
        int rating_num = rand_rating();
        fx_net_nominal[idx[rating_num][rand() % idx[rating_num].size()]] += fx[i].nominal;
    }

    // SWAP

    for (unsigned long i = 0; i < in.num_swap; ++i) {
        int rating_num = rand_rating();
        int rand_cp = idx[rating_num][rand() % idx[rating_num].size()];
        std::vector<float>::iterator it_fixed = (swap[i].ccy == "USD") ? usd_cash_flows.begin() : eur_cash_flows.begin();
        std::vector<float>::iterator it_floating = (swap[i].ccy == "USD") ? usd_floating.begin() : eur_floating.begin();
        unsigned first = rand_cp * in.tenor[1]*12;
        unsigned last = first + swap[i].tenor;
        std::for_each(it_fixed + first, it_fixed + last, add(swap[i].nominal * swap[i].rate / 12.));
        *(it_fixed + last - 1) += swap[i].nominal; // adding principal to both legs makes no difference
        std::for_each(it_floating + first, it_floating + last, add(-swap[i].nominal));
    }
}
