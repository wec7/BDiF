/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: ScrubParams Class methods implementation
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

// C++ include

#include <sys/time.h>

// local include

#include "timer.h"

void Timer::start() { 
	/* @summary: assign tv_start as current moment */
	gettimeofday(&tv_start, NULL); 
}

void Timer::end() {
	/* @summary: assign tv_end as the current moment */
    gettimeofday(&tv_end, NULL);
    d_time += double(tv_end.tv_sec - tv_start.tv_sec) + \
    double(tv_end.tv_usec - tv_start.tv_usec) / 1000000.;
}