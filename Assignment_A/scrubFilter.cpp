/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: ScrubFilter to classify signal and noise
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

// local include

#include "scrubFilter.h"

// C++ includes

#include <algorithm> 
#include <cmath>

using namespace std;

void ScrubFilter::filter_dateTypo(vector<DataTick> &data) {
    /*
    @summary: filter datatick with date typos, should be executed before filter_duplicate
    @param data: vector of DataTick objects containing the data
    @param sizet_lines: number of lines in this vector
    */

    // initialize id counter

    size_t sizet_lines = data.size();
    int i_true = -1; 
    
    // find 1st signal position

    for (size_t i = 0; i < sizet_lines; ++i) {
        if (!data[i].b_signal) 
            continue;
        i_true = i;
        break;
    }

    // all noise case

    if (i_true == -1) return;

    int i_second = -1; 
    for (size_t i = i_true + 1; i < sizet_lines; ++i) {
        if (!data[i].b_signal) continue;
        i_second = i;
        break;
    }
    if (i_second == -1) return;
    if (data[i_true].i_date != data[i_second].i_date) {
        int i_third = -1;
        for (size_t i = i_second + 1; i < sizet_lines; ++i) {
            if (!data[i].b_signal) continue;
            i_third = i;
            break;
        }
        if (i_third == -1) return;
        if (data[i_true].i_date != data[i_third].i_date) {
            data[i_true].b_signal = false;
            i_true = i_second;
        }
    }

    // filter the i_date-typo type noise

    for (size_t i = i_true + 1; i < sizet_lines; ++i) {
        if (!data[i].b_signal) continue;
        if ((data[i].i_date != data[i_true].i_date) && (data[i].i_hour == data[i_true].i_hour)) {
            data[i].b_signal = false;
            continue;
        }
        i_true = i;
    }
}

void ScrubFilter::filter_duplication(vector<DataTick> &data) {
    /* 
    @summary: sort datatick and remove duplication by comparing two neighbourhood values
    @param data: vector of DataTick objects containing the data
    @param sizet_lines: number of lines in this vector
    */

    // qicksort datatick

    size_t sizet_lines = data.size();
    sort(data.begin(), data.begin() + sizet_lines);

    // find 1st signal position

    int i_true = -1;
    for (size_t i = 0; i < sizet_lines; ++i) {
        if (!data[i].b_signal) continue;
        i_true = i;
        break;
    }
    
    // all noise case

    if (i_true == -1) 
        return;

    // filter the duplicated noise

    for (size_t i = i_true + 1; i < sizet_lines; ++i) {
        if (!data[i].b_signal) continue;
        if (data[i] == data[i_true]) {
            data[i].b_signal = false;
            continue;
        }
        i_true = i;
    }
}

void ScrubFilter::filter_invalidPrice(vector<DataTick> &data, float tol, unsigned sample_size) {
    /* 
    @summary: filter invalid price according to normal distribution
    @param tol: tolerance to decide how much data to kick out
    @param sample_size: default 100
    @reference: this part referenced from Yu Gan
    */

    // initialization

    double mean = 0;
    double stdev = 0;
    size_t sizet_lines = data.size();

    if (sizet_lines <= 2 * sample_size) {
        for (unsigned i = 0; i < sizet_lines; ++i) {
            if (!data[i].b_signal) continue;
            mean += data[i].f_price;
            stdev += data[i].f_price * data[i].f_price;
        }
        mean = mean / sizet_lines;
        stdev = sqrt(stdev / sizet_lines - mean * mean);
    } else {
        for (unsigned i = 0; i < sample_size; ++i) {
            if (!data[i].b_signal) continue;
            mean += data[i].f_price;
            stdev += data[i].f_price * data[i].f_price;
        }
        for (unsigned i = sizet_lines - 1; i >= sizet_lines - sample_size; --i) {
            if (!data[i].b_signal) continue;
            mean += data[i].f_price;
            stdev += data[i].f_price * data[i].f_price;
        }
        mean = mean / sample_size / 2.;
        stdev = sqrt(stdev / sample_size / 2. - mean * mean);
    }

    // calculate the upper and lower bound for the f_price

    double upperbound = mean + tol * stdev;
    double lowerbound = mean - tol * stdev;

    // filter the wrong f_price type noise

    for (size_t i = 0; i < sizet_lines; ++i) {
        if (!data[i].b_signal) continue;
        if ((data[i].f_price > upperbound) || (data[i].f_price < lowerbound)) {
            data[i].b_signal = false;
        }
    }
}

