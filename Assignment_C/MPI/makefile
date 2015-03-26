# Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
# Description: Makefile to run C++11 with Open MPI
# Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu

CC=mpic++
CFLAGS=-Wall -std=c++11 -O3

SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)
DEP=$(SRC:.cpp=.d)
DEP_TEMP=$(SRC:.cpp=.d.*)

LIB=$(shell mpic++ --showme:link) -I/share/apps/boost/1.56.0/lib
INCLUDE=$(shell mpic++ --showme:compile) -I/share/apps/boost/1.56.0/include

grep_cmd = grep -l '^[ \t\v]*[^/]*.*\<main[ \t\v]*(' $(SRC)
EXE = $(basename $(shell $(grep_cmd)))
ifneq ($(words $(EXE)),1)
$(error The main function can be defined exactly once)
endif

all:$(EXE).x

$(EXE).x:$(OBJ)
	$(CC) $(OBJ) -o $@ $(CFLAGS) $(LIB)

-include $(DEP)

%.d:%.cpp
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.o:%.cpp
	$(CC) $< -c -o $@ $(CFLAGS) $(INCLUDE)

clean:
	rm -rf $(EXE).x $(OBJ) $(DEP) $(DEP_TEMP)