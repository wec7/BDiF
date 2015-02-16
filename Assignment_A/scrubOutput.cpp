/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: ScrubOutput Class, to output log stream for ScrubLogFile class
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#include "scrubOutput.h"

FILE*& ScrubOutput::file() {
	/* method used for filestream initialization */
    static FILE* pFile = stderr;
    return pFile;
}

void ScrubOutput::output(const std::string& s_msg) {
	/*
	@summary: ouput string message to stream
	@param msg: output string message
	*/
    FILE* pFile = file();
    if (!pFile) 
    	return;
    fprintf(pFile, "%s", s_msg.c_str());
    fflush(pFile);
}
