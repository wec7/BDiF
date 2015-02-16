/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: ScrubLogFile Class, for write log
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef SCRUBLOGFILE_H
#define SCRUBLOGFILE_H
#define LOG(level) if(level <= ScrubLogFile::ReportLevel()) ScrubLogFile().Get(level)

// C++ includes

#include <string>
#include <cstdio>
#include <iomanip>
#include <sstream>

// Enumerator for Log levels

enum LogLevel {OFF, DIAG, INFO};

std::string levelToString(LogLevel level);
LogLevel stringToLevel(std::string str);
std::string now();

// ScurbLogFile Class

class ScrubLogFile {
    /*
    @summary: scrubLogFile to save info and write log
    @mem count: counter to count how many instances of this class 
    @mem ostringstream: output stream 
    */
    public:
        // members

        static int count; 
        std::ostringstream os;

        // constructor and destructor

        ScrubLogFile();
        ScrubLogFile(const ScrubLogFile&);
        virtual ~ScrubLogFile();

        // operator overloading

        ScrubLogFile& operator = (const ScrubLogFile&);

        // methods

        static LogLevel& ReportLevel();
        std::ostringstream& Get(LogLevel level);
};

#endif
