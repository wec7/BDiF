/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Swap Class, used to define swap class
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef SWAP_H
#define SWAP_H

// C++ includes
#include <vector>
#include <string>

// local include
#include "params.h"

class Swap{
	/*
	@summary: swap instrument
	@mem cry: currency
	@mem nominal: long (receive fixed) : positive nominal, short (receive floating) : negative nominal
	@mem rate: interest rate of fixed leg
	@mem tenor: in months, SWAP_TENOR in Params is in years
	*/
    public:
    std::string ccy;
    float nominal; 
    float rate;
    int tenor;
    
    Swap(float f_nominal): nominal(f_nominal) {};
    Swap(std::string str_ccy, float f_nominal, float f_rate, int t): ccy(str_ccy), nominal(f_nominal), rate(f_rate), tenor(t){};
};

std::vector<Swap> create_swaps(Params& in);
std::vector<Swap> create_fxs(Params& in);

#endif
