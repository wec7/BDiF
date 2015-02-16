/*
Copyright: Copyright (C) 2015 Baruch College - All Rights Reserved
Description: DataTick Class 
Author: Weiyi Chen, weiyi.chen@baruchmail.cuny.edu
*/

#ifndef DATATICK_H
#define DATATICK_H

class DataTick {
    /* 
    @summary: DataTick class represents a data line from data file 
    @mem c_line: length of raw data line
    @mem b_signal: indicate whether this data line is signal or noise
    @mem i_date: save data line's date, e.g. 20140804
    @mem i_hour: data line's hour, e.g. 10
    @mem i_minute: data line's minute, e.g. 00
    @mem f_second: data line's second, e.g. 13.281486
    @mem f_price: data line's price, e.g. 782.83
    @mem i_volume: data line's volume, e.g. 443355
    */
public:
    char* c_line;
    int i_length; 
    bool b_signal;

    int i_date, i_hour, i_minute;
    double f_second;
    double f_price;
    int i_volume;

    bool operator < (const DataTick&) const;
    bool operator == (const DataTick&) const;

    void parse(char*);
    template<typename T> void parse_helper(int offset, int len, T& value);
};

#endif
