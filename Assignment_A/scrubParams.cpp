/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: ScrubParams Class methods implementation
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

// C++ includes

#include <boost/algorithm/string.hpp>
#include <vector>
#include <cstdlib>

// Local includes

#include "scrubParams.h"

void ScrubParams::scrubParams_parser(std::ifstream& infile) {
    /* 
    @summary: parser to read command line paramters from a infile
    @param infile: input file, usually named "params.txt"
    */
    std::string s_line;
    std::vector<std::string> v_keys;

    while (getline(infile, s_line)) {
        boost::split(v_keys, s_line, boost::is_any_of(","));
        if (v_keys[0] == "-log")
            logFile = v_keys[1];
        else if (v_keys[0] == "-data")
            dataFile = v_keys[1];
        else if (v_keys[0] == "-signal")
            signalFile = v_keys[1];
        else if (v_keys[0] == "-noise")
            noiseFile = v_keys[1];
        else if (v_keys[0] == "-tolerance")
            tol = atof(v_keys[1].c_str());
    }
}
