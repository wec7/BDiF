/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: ScrubOutput Class, to output log stream for ScrubLogFile class
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef SCRUBOUTPUT_H
#define SCRUBOUTPUT_H

// C++ include

#include <sstream>

class ScrubOutput {
	/* stream-like output class for ScrubLogFile class */
public:
    static FILE*& file();
    static void output(const std::string& msg);
};

#endif