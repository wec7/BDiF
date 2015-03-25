//============================================================================
// Name        : Assignment_A_normal.cpp
// Author      : Luxi Tang
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <boost/algorithm/string.hpp>
#include "../../src/scrub_class.h"
#include "normal_class.h"
#include "normal_functions.h"
using namespace std;

#ifndef CRITICAL_VALUE
#define CRITICAL_VALUE 0.001
#endif

#define TIMESPOT std::chrono::system_clock::time_point
#define TIMENOW std::chrono::system_clock::now()
#define TIMESLOT std::chrono::duration_cast<std::chrono::milliseconds> //need to add (time_t - time_0).count() when using

int main(int argc, char** argv)
{
	/*
	 * step 1: read file into memory, sort as a vector, calculate return and log return
	 */
	string input_path;
	//input_path = argv[0];
	input_path ="signal.txt";
	vector<Message> message_v;
	ifstream fin;
	fin.open(input_path.c_str());
	string tmp_line;
	if (!fin.is_open())
	{
		cerr << "Can't open " << input_path << " file for input.\n";
		exit(EXIT_FAILURE);
	}
	string line;
	while(getline(fin, line)){
		int size = line.size();
		std::vector<std::string> strs;
		boost::split(strs, line, boost::is_any_of("\n ,"));
		//cout << strs[0] << endl;
		string temp_timestamp;
		float temp_price;
		int temp_vol;
		temp_timestamp = strs[0];//line.substr(0, 24);
		temp_price = stof(strs[1]);//line.substr(25, 7));
		temp_vol = stoi(strs[2]);//line.substr(33, 6).c_str());
		Message temp_msg(temp_timestamp, temp_price, temp_vol);
		//test_1(temp_msg);
		message_v.push_back(temp_msg);
		//cout << temp_msg.gettime() << endl;
	}
	fin.close();
	cout << "Successfully read the file.\n";


	sort(message_v.begin(), message_v.end());
	vector<double> p_return;
	int it = 0;
	//cout << getReturn(message_v[1], message_v[25]) << endl;
	vector<double> log_p_return;
	long _size = message_v.size() - 1;
	//cout << _size << endl;
	//cout << message_v[0].getprice();
	//cout << p_return.size();
	for (int it = 0; it < _size; it++)
	{
		p_return.push_back(getReturn(message_v[it], message_v[it+1]));
		log_p_return.push_back(getLogReturn(message_v[it], message_v[it+1]));
	}
	//cout << p_return.size() << endl;
	//cout << p_log_return.size() << endl;/**/
	//check the size of two vectors, should be the same.
	//otherwise through error message to log file.

	/*
	 * step 2:calculate sample mean of subset
	 */
	StatsObject return_stats;
	cout << "For the price return (percentage, microsecond as time unit):" << endl;
	return_stats.normalityTest(p_return);
	if (return_stats._JB_factor < CRITICAL_VALUE)
		cout << "This sample follows normal distribution." << endl;
	else
		cout << "This sample does not follow normal distribution." << endl;
	cout << "For the the price return (microsecond as time unit):" << endl;
	StatsObject log_return_stats;
	log_return_stats.normalityTest(log_p_return);
	if (log_return_stats._JB_factor < CRITICAL_VALUE)
		cout << "This sample follows log 	normal distribution." << endl;
	else
		cout << "This sample does not follow log normal distribution." << endl;
	//cout << log_return_stats._sample_sum;

	/*
	 * step 3:calculate skewness, kurtosis, and JB of subset
	 */

	/*
	 * step 4: compare JB to critical value, output the conclusion
	 */

	return 0;
}
