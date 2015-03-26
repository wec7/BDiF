/* 
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Data test - run the function
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

// C++ includes

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <random>

// local includes

#include "DataParams.hpp"
#include "DataTest.hpp"
using namespace std;

int main()
{
	/* main function for makefile */
    generate_data();
    return 0;
}

void generate_data() {
	/* function implementation to generate synthetic data */
    
	// pass in dataparameters

    DataParams params;

    // track size

   	ifstream ifs(params.inputData, ifstream::binary);
   	ifs.seekg(0, ifs.end);
	int i_inputSize = ifs.tellg();
	ifs.close();

    int i_numPoints = params.size / i_inputSize;
    double d_delta = sqrt(1.0/i_numPoints) * params.sigma;

    // open input and output file

    string str_prevLine;
    string str_currLine;

    ifstream in(params.inputData);
    getline(in, str_prevLine);
    ofstream out;
    out.open(params.outputData, ios::app);

    // random number generator

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator (seed);
    normal_distribution<float> standardNormal(0.0,1.0);

    // read and write data

    while(getline(in, str_currLine))
    {
        float f_prev = stof(str_prevLine);
        float f_curr = stof(str_currLine);
        float driftPerPoint = (f_curr-f_prev) / i_numPoints;

        out << to_string(f_prev) << endl;
        for(int i = 0; i < i_numPoints; i++) {
            f_prev += driftPerPoint + d_delta * standardNormal(generator);
            out << to_string(f_prev) << endl;
        }
        out << to_string(f_curr) << endl;
        str_prevLine = str_currLine;
    }

    // close files

    in.close();
    out.close();
}