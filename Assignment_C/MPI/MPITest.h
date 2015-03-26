/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: MPI test functions
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef MPITEST_H
#define MPITEST_H

#include "LogFile.h"

typedef LogFile FILELog;
vector<double> cal_property(int i_nodeRank, int blockSize);
MPIParams params;

#endif