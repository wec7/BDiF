/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: Normality Class implementation
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

// local includes

#include "normality.h"

// C++ includes

#include <iostream>
#include <math.h>
using namespace std;


double Normality::get_sum(vector<double>& vec) {
	/* derive the sum of normality */
	while (count < vec.size()) {
		sum = sum + vec[count];
		count ++;
	}
	return sum;
}

double Normality::get_mean() {
	/* derive the mean from sum */
	mean = sum / (double)count;
	return mean;
}

void Normality::update(vector<double>& vec)
{
	/* update stats info inside the class */
	for (int i = 0; i < count; i++) {
		double temp_std = vec[i] - mean;
		var = pow(temp_std, 2.0) + var;
		skew = pow(temp_std, 3.0) + skew;
		kurt = pow(temp_std, 4.0) + kurt;
	}
	var = var / count;
	skew = skew / count;
	kurt = kurt / count;
	skew = skew/ (pow (var, 1.5));
	kurt = kurt/ (pow (var, 2.0));
}

double Normality::get_JB_factor() {
	/* calculate JB factor */
	JB_factor = kurt - 3.0;
	JB_factor = pow(JB_factor, 2.0);
	JB_factor = JB_factor / 4.0;
	JB_factor = JB_factor + pow(skew, 2.0);
	JB_factor = JB_factor * count;
	JB_factor = JB_factor / 6.0;
	return JB_factor;
}

Normality::Normality(){
	/* initialization constructor */
	mean = 0;
	sum = 0;
	count = 0;
	var = 0;
	skew = 0;
	kurt = 0;
	JB_factor = 0;
}

Normality::Normality(vector<double>& vec){
	/* complicated constructor */
	sum = get_sum(vec);
	mean = get_mean();
	update(vec);
	JB_factor = get_JB_factor();
}