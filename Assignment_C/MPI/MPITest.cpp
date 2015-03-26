/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: MPI test
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

// C++ includes

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <vector>

// local includes

#include "MPIParams.h"
#include "Output.h"
#include "MPITest.h"
#include "Timer.h"

using namespace std;

int main (int argc, char* argv[])
{
    // set up log file, input data file

	Output::file() = fopen(params.log_file, "a");
    ifstream in(params.data_file);
    in.seekg(0, in.end);

    // set up MPIs

    int i_nodeRank, i_numMPI;
    MPI_Status status;
    MPI_Init(&argc, &argv);     					
    MPI_Comm_rank(MPI_COMM_WORLD, &i_nodeRank);      
    MPI_Comm_size(MPI_COMM_WORLD, &i_numMPI); 

    // write logs

	LOG(OFF) << "Node " << i_nodeRank << " starts...";

    // timer setting, reference from Zhenfeng

	Timer timer;
	timer.start();

    // main part to calculate mean and variance

	if(i_nodeRank != 0){
        vector<double> vec_property = cal_property(i_nodeRank, in.tellg() / i_numMPI);
        double property[3] = {vec_property[0],vec_property[1],vec_property[2]};
        MPI_Send(property, 3, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    else {
        double property[3] = {0,0,0};
    	for(int source = 1; source < i_numMPI; source++) {
    		double prop_temp[3];
			MPI_Recv(prop_temp, 3, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, &status);
			for(int i = 0; i < 3; i++) property[i] += prop_temp[i];
    	}
    	double mean = property[1] / property[0];
    	double variance = property[2] / property[0] - mean * mean;
    	LOG(INFO) << "Mean: " << mean << " Variance: " << variance;
    }

    // timer ends

	timer.end();

    // print out results to log file

	LOG(OFF) << "Node " << i_nodeRank << " ends...";
	LOG(OFF) << "Node " << i_nodeRank << " takes " << timer.d_time << "s to run." ;

    // MPI, files close

    in.close();
	MPI_Finalize();

    return 0;
}

vector<double> cal_property(int i_nodeRank, int blockSize) {
    /* 
    @summary: calculate properties as of count, d_sum, square d_sum 
    @param i_nodeRank: indicate which MPI is running
    @param blockSize: size of data for each MPI
    @return: vector of properties, includes count, sum, and square sum
    */
    vector<double> vec_property;

    ifstream in(params.data_file);
    in.seekg(i_nodeRank * blockSize, in.beg);

    int i_prevLine = in.tellg();
    string line;
    getline(in, line);
    int i_currLine;

    double d_sum = 0, d_sum_sq = 0;
    int i_count = 0;

    while (getline(in, line) && (i_currLine = in.tellg()) - i_prevLine < blockSize) {
        double data = atof(line.c_str());
        d_sum += data;
        d_sum_sq += data * data;
        i_count++;
    }

    vec_property.push_back(i_count);
    vec_property.push_back(d_sum);
    vec_property.push_back(d_sum_sq);

    return vec_property;
}

