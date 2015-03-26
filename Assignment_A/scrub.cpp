/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Scrub functions for main file
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

// C++ includes

#include <mpi.h>
#include <cstdio> 
#include <vector>
#include <cstring> 
#include <cstdlib> 
#include <unistd.h>

// local includes

#include "timer.h"
#include "datatick.h"
#include "scrub.h"
#include "scrubOutput.h"

// global parameters

void read_params(ScrubParams& params, bool& b_paramsFileOpen){
    /*
    @summary: read parameters from configfile to params object
    @param params: ScrubParams object to hold parameters
    @param b_paramFileOpen: indicate whether params.txt opens successfully
    */
    std::ifstream configfile("params.txt");
    if (configfile.is_open()) {
        b_paramsFileOpen = true;
        params.scrubParams_parser(configfile); // Parse the configuration file
        configfile.close();
    }
}

void write_log(int mpi_rank, int mpi_size, bool b_paramsFileOpen, ScrubParams params, int argc, char **argv) {
    /*
    @summary: write log before running programs
    @param mpi_rank: decide whether to write log
    @param mpi_size: MPI number
    @param b_paramsFileOpen: indicate whether params.txt opened successfully
    @param params: ScrubParams object holding parameters
    @param argc, argv: used to find path and executing programs
    */
    if (mpi_rank == 0) {
        LOG(OFF) << "EXECUTE\t" << argv[0];
        if (b_paramsFileOpen) {
            LOG(INFO) << "CONFIGFILE\tparams.txt";
        } else {
            LOG(DIAG) << "Unable to open Configuration File: params.txt.";
            LOG(INFO) << "Default parameters used.";
        }
        LOG(INFO) << "PRINTNUM\tMPI\t" << mpi_size;
    }

    if (argc != 2) {
        if (mpi_rank == 0)
            LOG(DIAG) << "Usage: " << argv[0] << " working_directory";
        MPI_Finalize();
        exit(0);
    }

    std::string s_pwd = argv[1];
    if (mpi_rank == 0) {
        remove((s_pwd + "/" + params.signalFile).c_str());
        remove((s_pwd + "/" + params.noiseFile).c_str());
    }
}

void open_files(MPI_File &in_data, MPI_File &out_signal, MPI_File &out_noise, std::string s_pwd, ScrubParams params, double mpi_rank) {
    /*
    @summary: open MPI files
    @param in_data: MPI file of data.txt as an input
    @param out_signal: MPI file of signal.txt as an output
    @param out_noise: MPI file of noise.txt as an output
    @param s_pwd: path of working directory
    @param params: parameter object to read other parameters
    @param mpi_rank: decide whether to write log
    */
    int ierr = MPI_File_open(MPI_COMM_WORLD, const_cast<char*>((s_pwd + "/" + params.dataFile).c_str()), MPI_MODE_RDONLY, MPI_INFO_NULL, &in_data);
    if (ierr) {
        if (mpi_rank == 0)
            LOG(DIAG) << "Couldn't Open File " << s_pwd + "/" + params.dataFile;
        MPI_Finalize();
        exit(0);
    }
    MPI_File_open(MPI_COMM_WORLD, const_cast<char*>((s_pwd + "/" + params.signalFile).c_str()), MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &out_signal);
    MPI_File_open(MPI_COMM_WORLD, const_cast<char*>((s_pwd + "/" + params.noiseFile).c_str()), MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &out_noise);
}

void read_data(MPI_File &in, const int rank, const int size, MPI_Status status, MPI_Offset &start_point, const long long &max, const int &overlap, char ***lines, size_t &nlines, char ** chunk) {
    /* @summary: read data */
    MPI_Offset filesize;
    MPI_Offset localsize;
    MPI_Offset start;
    MPI_Offset end;

    /* figure out who reads what */
    MPI_File_get_size(in, &filesize);
    localsize = (start_point + max * (size + 1) > filesize) ? ((filesize - start_point) / size + 1) : max;
    start = start_point + localsize * rank;
    end = start + localsize - 1;

    /* add overlap to the end of everyone's chunk... */
    end += overlap;

    /* when at the end of file */
    if (end > filesize) end = filesize;

    localsize = end - start + 1;

    /* everyone reads in their part */
      MPI_Datatype type_read;    
    MPI_Type_contiguous(localsize, MPI_CHAR, &type_read);
    // Datatype should be commited before using in communication (MPI_File_set_view)
    MPI_Type_commit(&type_read);
      MPI_File_set_view(in, start, MPI_CHAR, type_read, const_cast<char *>("native"), MPI_INFO_NULL);
      MPI_File_read(in, *chunk, 1, type_read, &status);
    (*chunk)[localsize] = '\0';

    /*
     * everyone calculate what their start and end *really* are by going 
     * from the first newline after start to the first newline after the
     * overlap region starts (eg, after end - overlap + 1)
     */

    int locstart = 0, locend = localsize;
    if (start != 0) {
        while((*chunk)[locstart] != '\n') ++locstart;
        ++locstart;
    }
    if (end != filesize) {
        locend -= overlap;
        while((*chunk)[locend] != '\n') ++locend;
    }
    localsize = locend - locstart + 1;
    
    /* point to the real chunk I want to read from */
    char * real_chunk = &((*chunk)[locstart]);
    real_chunk[localsize] = '\0';

    /* Now we'll count the number of lines */
    nlines = 0;
    for (int i = 0; i < localsize; ++i)
        if (real_chunk[i] == '\n') ++nlines;

    /* Now the array lines will point into the data array at the start of each line */
    /* assuming nlines > 1 */
    (*lines)[0] = strtok(real_chunk, "\n");
    for (size_t i = 1; i < nlines; ++i)
        (*lines)[i] = strtok(NULL, "\n");

    /* determine new starting point */
    if (rank == size - 1)
      start_point = (end == filesize) ? -1 : end - overlap + 1;

    return;
}

void parse_data(size_t sizet_lines, std::vector<DataTick> &dataticks, char **lines) {
    /*
    @summary: parse data 
    @param sizet_lines: size of lines in data.txt
    @param dataticks: a vector holding datatick instances
    @param lines: address to the raw data datatick
    */
    for (size_t i = 0; i < sizet_lines; ++i)
        dataticks[i].parse(lines[i]);
}

void scrub_data(size_t sizet_lines, std::vector<DataTick> &dataticks, ScrubParams params){
    /*
    @summary: follow three ways to scrub data
    @param sizet_lines: size of lines in data.txt
    @param dataticks: a vector holding datatick instances
    @param params: parameter object to read other parameters
    */
    if (sizet_lines != 0) {
        ScrubFilter filter;
        filter.filter_dateTypo(dataticks);
        filter.filter_duplication(dataticks); 
        filter.filter_invalidPrice(dataticks, params.tol);
    }
}


