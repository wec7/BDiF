/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: LogFile Class, for write log
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#include "logFile.h"
#include "output.h"

// LogLevel enumerator

std::string levelToString(LogLevel level) {
	/*
	@summary: convert level to string
	@param level: level to convert
	@return: a string corresponds to the level
	*/
    static const char* const buffer[] = {"CALL", "DIAG", "INFO"};
    return buffer[level];
}

LogLevel stringToLevel(std::string str) {
	/*
	@summary: convert string to log level
	@param str: keyword string to convert
	@return: INFO by default
	*/
    if (str == "CALL")
        return OFF;
    if (str == "DIAG")
        return DIAG;
    if (str == "INFO")
        return INFO;
    return INFO;
}

std::string now() {
	/* 
	@summary: Get current date and time, format is YYYY-MM-DD,HH:mm:ss 
	@reference: http://www.cplusplus.com/reference/ctime/strftime/
	*/
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, 80, "%Y%m%d:%X", timeinfo);
    return buffer;
}

// LogFile Class 

int LogFile::count = 0;

LogFile::LogFile() {
	/* counter add 1 when a new instance of this object creates */
    count += 1;
}

LogFile::~LogFile() {
	/* destructor to let Output receieve the results  */
    os << std::endl;
    Output::output(os.str());
}

LogLevel& LogFile::ReportLevel() {
	/* return the level infomation */
    static LogLevel level = INFO;
    return level; 
}

std::ostringstream & LogFile::Get(LogLevel level) {
	/* 
	@summary: pass content to ostream
	@param level: corresonded level output
	*/
    os << std::setfill('0') << std::setw(8) << count;
    os << "\t" << now();
    os << "\t" << levelToString(level) << "\t";
    return os;
}
