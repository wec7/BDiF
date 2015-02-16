/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: ScrubParams Class methods definition
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef SCRUBPARAMS_H
#define SCRUBPARAMS_H

// C++ imports

#include <string>
#include <fstream>
using namespace std;

class ScrubParams {
    /*
    @summary: Class to take command line params, or parameter file
    @mem logFile: output log filename
    @mem dataFile: input data filename
    @mem signalFile: output signal filename
    @mem noiseFile: output noise filename
    @mem tol: tolerance to accept datatick as a signal
    */
public:
    // class members
    string logFile;
    string dataFile;
    string signalFile;
    string noiseFile;
    float tol; 

    // constructor and initialization list
    ScrubParams(): 
    logFile("log.txt"),
    dataFile("data.txt"),
    signalFile("signal.txt"),
    noiseFile("noise.txt"),
    tol(10.0)
    {} 

    // parser method
    void scrubParams_parser(ifstream& infile);
};



#endif
