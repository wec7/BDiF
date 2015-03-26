/* 
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Data parameter class implementation
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

// local includes

#include "DataParams.hpp"

DataParams::DataParams() {
	/* DataParams constructor */
	inputData = "input.csv";
    outputData = "output.csv";
    size = 45000000000;
    sigma = 1.95;
}