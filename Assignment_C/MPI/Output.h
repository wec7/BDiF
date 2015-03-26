/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Output Class, to output log stream for LogFile class
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef OUTPUT_H
#define OUTPUT_H

// C++ include
#include <sstream>

class Output {
	/* stream-like output class for ScrubLogFile class */
public:
    static FILE*& file();
    static void output(const std::string& msg);
};

#endif