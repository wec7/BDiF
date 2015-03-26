/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: MPI paramter class
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef MPIPARAMS_H
#define MPIPARAMS_H

#include <string>
using namespace std;

class MPIParams
{
public:
    string data_file;
    const char* log_file;

    MPIParams();
};

#endif