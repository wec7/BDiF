/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Normality Class with its method
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef NORMALITY_H_
#define NORMALITY_H_

// C++ includes
#include <vector>
using namespace std;

class Normality{
	/* normality stats class for data normality test */

public:

	// data
	double mean;
	double sum;
	long count;
	double var;
	double skew;
	double kurt;
	double JB_factor;

	// methods
	double get_sum(vector<double>& return_v);
	double get_mean();
	void update(vector<double>& return_v);
	double get_JB_factor();

	// constructors
	Normality();
	Normality(vector<double>& return_v);
};


#endif /* NORMALITY_H_ */
