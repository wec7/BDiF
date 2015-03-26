/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: LogFile Class, for write log
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef LOGFILE_H
#define LOGFILE_H
#define LOG(level) if(level <= LogFile::ReportLevel()) LogFile().Get(level)

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

class LogFile{
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

        LogFile();
        LogFile(const LogFile&);
        virtual ~LogFile();

        // operator overloading

        LogFile& operator = (const LogFile&);

        // methods

        static LogLevel& ReportLevel();
        std::ostringstream& Get(LogLevel level);
};

#endif
