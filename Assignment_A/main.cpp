/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Main file to scrup data and output normal statistics
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

const long long MAX_LINE = 20000;
const long long MAX_BYTE = sizeof(char) * MAX_LINE * 50;

int main(int argc, char **argv) {

    // MPI initialization, assign rank and size value 

    MPI_Status mpi_status;
    MPI_Init(&argc, &argv);

    int mpi_rank, mpi_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    // read parameters from params.txt and argv

    std::string s_pwd = argv[1];
    ScrubParams params;
    bool b_paramsFileOpen = false;
    read_params(params, b_paramsFileOpen);
    
    // set up LogFile board and writer

    ScrubOutput::file() = fopen(params.logFile.c_str(), "a");
    write_log(mpi_rank, mpi_size, b_paramsFileOpen, params, argc, argv);

    // open data files to read and write: data.txt, signal.txt, noise.txt

    MPI_File in_data, out_signal, out_noise;
    open_files(in_data, out_signal, out_noise, s_pwd, params, mpi_rank);

    // set up timers before main part

    Timer t_read;
    Timer t_parse;
    Timer t_scrub;
    Timer t_write_signal;
    Timer t_write_noise;

    // main part initialization

    char *ptr_read = (char*) malloc(MAX_BYTE * 3 * sizeof(char));
    char **lines = (char **) malloc(MAX_LINE * 3 * sizeof(char *));

    char *ptr_writesignal = (char *) malloc(MAX_BYTE * 3 * sizeof(char));
    char *prt_writenoise = (char *) malloc(MAX_BYTE * sizeof(char));

    size_t sizet_lines;
    DataTick dt_empty;
    std::vector<DataTick> dataticks(MAX_LINE * 3, dt_empty);

    // set up MPI before main part

    MPI_Offset offset_start = 0;
    MPI_Offset offset_signal = 0, offset_noise = 0;
    MPI_Offset offset_signalDynamic = 0, offset_noiseDynamic = 0;

    long long len_singals[mpi_size];
    long long len_noises[mpi_size];

    MPI_Datatype type_signal, type_noise;

    while (offset_start != -1) {
        
        // read data

        t_read.start();
        read_data(in_data, mpi_rank, mpi_size, mpi_status, offset_start, MAX_BYTE, 100, &lines, sizet_lines, &ptr_read);
        t_read.end();
        
        // parse data

        t_parse.start();
        parse_data(sizet_lines, dataticks, lines);
        t_parse.end();

        // scrub data

        t_scrub.start();
        scrub_data(sizet_lines, dataticks, params);
        t_scrub.end();

        // seperate noise and signal to different pointer

        long long len_signal = 0, len_noise = 0;
        char *ptr_signal = ptr_writesignal;
        char *ptr_noise = prt_writenoise;
        for (size_t i = 0; i < sizet_lines; ++i) {
            dataticks[i].c_line[dataticks[i].i_length] = '\n';
            int len = dataticks[i].i_length + 1;
            if (dataticks[i].b_signal) {
                len_signal += len;
                memcpy(ptr_signal, dataticks[i].c_line, len);
                ptr_signal += len;
            }
            else {
                len_noise += len;
                memcpy(ptr_noise, dataticks[i].c_line, len);
                ptr_noise += len;
            }
        }

        MPI_Bcast(&offset_start, 1, MPI_LONG_LONG_INT, mpi_size - 1, MPI_COMM_WORLD);
        MPI_Allgather(&len_signal, 1, MPI_LONG_LONG, len_singals, 1, MPI_LONG_LONG, MPI_COMM_WORLD);
        MPI_Allgather(&len_noise, 1, MPI_LONG_LONG, len_noises, 1, MPI_LONG_LONG, MPI_COMM_WORLD);
        
        // set the offset to decide where to start

        offset_signal = offset_signalDynamic;
        offset_noise = offset_noiseDynamic;
        for (int i = 0; i < mpi_rank; ++i) {
            offset_signal += len_singals[i];
            offset_noise += len_noises[i];
        }

        // set writing configs

        MPI_Type_contiguous(len_signal, MPI_CHAR, &type_signal);
        MPI_Type_contiguous(len_noise, MPI_CHAR, &type_noise);
        MPI_Type_commit(&type_signal);
        MPI_Type_commit(&type_noise);
        MPI_File_set_view(out_signal, offset_signal, MPI_CHAR, type_signal, const_cast<char *>("native"), MPI_INFO_NULL);
        MPI_File_set_view(out_noise, offset_noise, MPI_CHAR, type_noise, const_cast<char *>("native"), MPI_INFO_NULL);

        // start writing

        t_write_signal.start();
        MPI_File_write(out_signal, ptr_writesignal, len_signal, MPI_CHAR, &mpi_status);
        t_write_signal.end();
        t_write_noise.start();
        MPI_File_write(out_noise, prt_writenoise, len_noise, MPI_CHAR, &mpi_status);
        t_write_noise.end();
        
        // updatestart points for signal.txt and noise.txt

        for (int i = 0; i < mpi_size; ++i) {
            offset_signalDynamic += len_singals[i];
            offset_noiseDynamic += len_noises[i];
        }
    }

    // calculate the max time of the threads for each process

    double d_read, d_parse, d_scrub, d_writeSignal, d_writeNoise;
    MPI_Reduce(&t_read.d_time, &d_read, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_parse.d_time, &d_parse, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_scrub.d_time, &d_scrub, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_write_signal.d_time, &d_writeSignal, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_write_noise.d_time, &d_writeNoise, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // write log

    if (mpi_rank == 0) {
        LOG(INFO) << "PRINTTIME\tREAD:\t" << std::setprecision(3) << std::fixed << d_read;
        LOG(INFO) << "PRINTTIME\tPARSE:\t" << std::setprecision(3) << std::fixed << d_parse;
        LOG(INFO) << "PRINTTIME\tSCRUB:\t" << std::setprecision(3) << std::fixed << d_scrub;
        LOG(INFO) << "PRINTTIME\tWRITE_SIGNAL:\t" << std::setprecision(3) << std::fixed << d_writeSignal;
        LOG(INFO) << "PRINTTIME\tWRITE_NOISE:\t" << std::setprecision(3) << std::fixed << d_writeNoise;
        LOG(OFF) << "FINISH\t" << argv[0];
    }

    // Free memory

    free(ptr_read);
    free(ptr_writesignal);
    free(prt_writenoise);
    free(lines);

    // close MPI file and finalize MPI

    MPI_File_close(&in_data);
    MPI_File_close(&out_signal);
    MPI_File_close(&out_noise);
    MPI_Finalize();

    return 0;
}