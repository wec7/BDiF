/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Main file to scrup data
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
    char *ptr_signal;
    char *ptr_noise;

    size_t sizet_lines;
    DataTick empty;
    std::vector<DataTick> dataticks(MAX_LINE * 3, empty);

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

        // write data

        long long len_signal = 0, len_noise = 0;
        ptr_signal = ptr_writesignal;
        ptr_noise = prt_writenoise;
        int len;
        for (size_t i = 0; i < sizet_lines; ++i) {
            dataticks[i].c_line[dataticks[i].i_length] = '\n';
            len = dataticks[i].i_length + 1;
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

    double read, parse, scrub, write_signal, write_noise;
    MPI_Reduce(&t_read.d_time, &read, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_parse.d_time, &parse, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_scrub.d_time, &scrub, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_write_signal.d_time, &write_signal, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_write_noise.d_time, &write_noise, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // write log

    if (mpi_rank == 0) {
        LOG(INFO) << "PRINTTIME\tREAD:\t" << std::setprecision(3) << std::fixed << read;
        LOG(INFO) << "PRINTTIME\tPARSE:\t" << std::setprecision(3) << std::fixed << parse;
        LOG(INFO) << "PRINTTIME\tSCRUB:\t" << std::setprecision(3) << std::fixed << scrub;
        LOG(INFO) << "PRINTTIME\tWRITE_SIGNAL:\t" << std::setprecision(3) << std::fixed << write_signal;
        LOG(INFO) << "PRINTTIME\tWRITE_NOISE:\t" << std::setprecision(3) << std::fixed << write_noise;
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
