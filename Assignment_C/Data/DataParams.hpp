/* 
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Data parameters
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef DATAPARAMS_HPP_INCLUDED
#define DATAPARAMS_HPP_INCLUDED

// C++ includes

#include <string>
#include <fstream>
#include <iostream>

class DataParams
{
	/*
	@summary: data paramters
	@param inputData: input data csv filename
	@param outputData: output data csv filename
	@param size: size of data
	@param sigma: daily sigma of data
	*/
public:
    std::string inputData;
    std::string outputData;
    long long size;
    double sigma;

    DataParams();
};

#endif // DATAPARAMS_HPP_INCLUDED
