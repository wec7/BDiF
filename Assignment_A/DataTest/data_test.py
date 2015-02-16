import pandas as pd
import datetime as dt
import re
from matplotlib.pyplot import *


def read_data(path):
	df = pd.read_csv(path, '\t')
	return df

def parse_time(df):
	ls = [] 
	for i in df.index:
		ls_time = re.findall(r"[\w']+", df['Time'][i])
		year, month, day = int(ls_time[0][:4]), int(ls_time[0][4:6]), int(ls_time[0][6:])
		hour, minute, second = int(ls_time[1]), int(ls_time[2]), int(ls_time[3])
		minisecond = int(ls_time[4])
		ls.append(dt.datetime(year,month,day,hour,minute,second,minisecond))
	df['Time'] = ls
	return df

def plot_data(df):
	df.plot('Price', 'Time')
	show()

def main():
	df = read_data('data100k.txt')
	df = parse_time(df)
	plot(df['Time'],df['Price'],'ro')
	show()



if __name__ == '__main__':
	main()