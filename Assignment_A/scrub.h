/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: functions of Main file to scrup data
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef SCRUB_H
#define SCRUB_H

// local include

#include "scrubParams.h"
#include "scrubLogFile.h"
#include "scrubFilter.h"
#include "datatick.h"
#include "scrub.h"
#include "scrubOutput.h"

// C++ include

#include <cstring> 

void read_params(ScrubParams& params, bool& b_paramsFileOpen);
void write_log(int mpi_rank, int mpi_size, bool b_paramsFileOpen, ScrubParams params, int argc, char **argv);
void open_files(MPI_File &in_data, MPI_File &out_signal, MPI_File &out_noise, std::string s_pwd, ScrubParams params, double mpi_rank);
void read_data(MPI_File &in, const int rank, const int size, MPI_Status status, MPI_Offset &start_point, const long long &max, const int &overlap, char ***lines, size_t &nlines, char ** chunk);
void parse_data(size_t sizet_lines, std::vector<DataTick> &dataticks, char **lines);
void scrub_data(size_t sizet_lines, std::vector<DataTick> &dataticks, ScrubParams params);

#endif