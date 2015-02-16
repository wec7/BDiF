/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: scrubFilter to classify signal and noise
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef SCRUBFILTER_H
#define SCRUBFILTER_H

#include "datatick.h"
#include <vector>

class ScrubFilter
{
	/* @summary: three ways to filter data */
public:
	void filter_dateTypo(std::vector<DataTick> &dataticks);
	void filter_duplication(std::vector<DataTick> &dataticks);
	void filter_invalidPrice(std::vector<DataTick> &dataticks, float tol, unsigned sample_size = 100);
};

#endif
