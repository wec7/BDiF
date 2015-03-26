/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: DataTick Class methods implementation
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

// C++ imports

#include <cstring>
#include <cstdlib>
#include <typeinfo>
#include <cmath>

// local import

#include "datatick.h"


bool DataTick::operator == (const DataTick &other) const {
    /*  
    @summary: DataTick == operator overloading 
    @param other: another DataTick to compare with
    @return: whether they are equal
    */

    return (i_date == other.i_date) && (i_hour == other.i_hour) && \
    (i_minute == other.i_minute) && (fabs(f_second - other.f_second) < 5e-7);      
};


bool DataTick::operator < (const DataTick &other) const {
    /*  
    @summary: DataTick < operator overloading 
    @param other: another DataTick to compare with
    @return: whether this data's time is earlier than the other
    */

    if (b_signal == false) 
        return false;
    else if (other.b_signal == false) 
        return true;
    else if (i_date != other.i_date)
        return i_date < other.i_date;
    else if (i_hour != other.i_hour)
        return i_hour < other.i_hour;
    else if (i_minute != other.i_minute)
        return i_minute < other.i_minute;
    return f_second < other.f_second;
};

template<typename T> void DataTick::parse_helper(int offset, int len, T& value) {
    /*
    @summary: DataTick parser helper, convert data piece to specific type
    @param offset: offset of line at that DataTick
    @param len: length of data piece
    @param value: type could be int or float, the return value
    */
    char* c_offset = &c_line[offset];
    char delimeter = c_line[offset+len]; 
    c_line[offset+len] = '\0';
    if (typeid(value) == typeid(int))
        value = atoi(c_offset);
    else
        value = atof(c_offset);
    c_line[offset+len] = delimeter;
}

void DataTick::parse(char *rawData) {
    /* 
    @summary: DataTick parser, convert raw data to time, price and volume 
    @param rawData: raw data line from data.txt file, 
    e.g. 20140804:10:00:13.281486,782.83,443355
    */

    // Constuct outlier parameter

    b_signal = true;
    c_line = rawData;
    i_length = strlen(c_line);

    /// Parse date as an integer, e.g. 20140804

    DataTick::parse_helper(0, 8, i_date);

    // Parse hour as an integer between 9 and 16, e.g. 10

    DataTick::parse_helper(9, 2, i_hour);

    // Parse minute as an integer

    DataTick::parse_helper(12, 2, i_minute);

    // Parse second as a float, e.g. 13.281486

    DataTick::parse_helper(15, 9, f_second);

    // Parse price as a float 

    DataTick::parse_helper(25, 6 + (c_line[31] != ','), f_price);

    // Parse volume as an integer 

    DataTick::parse_helper(32 + (c_line[31] != ','), i_length-1, i_volume);

    // Verify hour is between 9 to 16

    if ((i_hour >= 16) || (i_hour <= 9))
        b_signal = false;

    // Verify volume and price is positive

    else if (i_volume <= 0 || f_price <= 0)
        b_signal = false;

    // Verify price is larger than 0

    else if (fabs(f_price) < 5e-7)
        b_signal = false;
}
