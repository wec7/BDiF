/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Timer Class, used to calculate running time in difference periods
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef TIMER_H
#define TIMER_H

// C++ includes

#include <sys/time.h>

class Timer {
    /*
    @summary: timer class to calculate time between start() and end()
    @mem tv_start: timeval object from sys/time.h, represent start time
    @mem tv_end: represent end time
    @mem time: time between start and end
    */
public:
    // members

    timeval tv_start;
    timeval tv_end;
    double d_time;

    // constructor with initialization

    Timer(): d_time(0) {}

    // methods

    void start(); 
    void end(); 
};

#endif
