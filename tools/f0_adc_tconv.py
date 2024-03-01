#!/usr/bin/python
# Generate overivew of possible sample rates of ADC

import os
import string
import sys

NUM_CHANNELS = 8

SAMPLE_TIMES = [
('ADC_SampleTime_1_5Cycles', 1.5),
('ADC_SampleTime_7_5Cycles', 7.5),
('ADC_SampleTime_13_5Cycles', 13.5),
('ADC_SampleTime_28_5Cycles', 28.5),
('ADC_SampleTime_41_5Cycles', 41.5),
('ADC_SampleTime_55_5Cycles', 55.5),
('ADC_SampleTime_71_5Cycles', 71.5),
('ADC_SampleTime_239_5Cycles', 239.5),
]

ADC_CLOCKS = [
('ADC_ClockMode_AsynClk', 14),
('ADC_ClockMode_SynClkDiv4', 12)
]

def t_conv(sample_time):
	return sample_time + 12.5

def calc_sample_rate(sample_time, clock):
	return (clock * 1000. * 1000.) / (NUM_CHANNELS * t_conv(sample_time))

def generate_tables():
	header = []
	header.append("NUM_CHANNELS=%d" % NUM_CHANNELS)
	table_header = []
	table_header.append(" %-26s" % ' ')
	for cn, c in ADC_CLOCKS:
		header.append("%s = %dMHz" % (cn, c))
		table_header.append("%24s" % cn)

	print ', '.join(header)
	print ' | '.join(table_header)
	print '-' * 81

	for tn, t in SAMPLE_TIMES:
		line = []
		line.append(" %-26s" % tn)
		for cn, c in ADC_CLOCKS:
			line.append("%21.02fKHz" % (calc_sample_rate(t, c) / 1000.))
		print ' | '.join(line)

def main(argv):
	generate_tables()

if __name__ == '__main__':
  main(sys.argv)