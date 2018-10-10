#!/usr/bin/python
# Generate overivew of possible sample rates of ADC

import os, argparse
import string, re
import sys

SYS_CLOCK = 168

SAMPLE_TIMES = [
('ADC_SampleTime_3Cycles', 3),
('ADC_SampleTime_15Cycles', 15),
('ADC_SampleTime_28Cycles', 28),
('ADC_SampleTime_56Cycles', 56),
('ADC_SampleTime_84Cycles', 84),
('ADC_SampleTime_112Cycles', 112),
('ADC_SampleTime_144Cycles', 144),
('ADC_SampleTime_480Cycles', 480),
]

ADC_PRESCALER = {
'ADC_Prescaler_Div2' : 2,
'ADC_Prescaler_Div4' : 4,
'ADC_Prescaler_Div6' : 6,
'ADC_Prescaler_Div8' : 8,
}

def t_conv(sample_time, two_sampling_delay_cycles):
  return sample_time + two_sampling_delay_cycles

def calc_sample_rate(clock, num_channels, sample_time, two_sampling_delay_cycles):
  return clock / (num_channels * t_conv(sample_time, two_sampling_delay_cycles))

def generate(adc_prescaler, two_sampling_delay, num_channels):

  m = re.match(r'ADC_TwoSamplingDelay_(?P<two_sampling_delay>\d+)Cycles', two_sampling_delay)
  two_sampling_delay_cycles = int(m.groupdict()['two_sampling_delay'])
  adc_prescaler_value = ADC_PRESCALER[adc_prescaler]

  clock = (SYS_CLOCK * 1000. * 1000.) / 2 / adc_prescaler_value

  header = []
  header.append("num_channels=%d" % num_channels)
  header.append("%dMHz / 2 / %d = %.2fMHz" % (SYS_CLOCK, adc_prescaler_value, clock / 1000. / 1000.))

  table = []
  for sample_time, cycles in SAMPLE_TIMES:
    table.append((sample_time, calc_sample_rate(clock, num_channels, cycles, two_sampling_delay_cycles) / 1000.))

  return (header, table)

def main(argv):
  adc_prescaler = argv[1]  
  two_sampling_delay = argv[2]
  num_channels = int(argv[3])
  if len(argv) > 4:
    min_rate = float(argv[4])
  else:
    min_rate = None

  header, table = generate(adc_prescaler, two_sampling_delay, num_channels)
  print '// %s' % " ".join(argv)
  print '// %s' % ', '.join(header)

  best_match = None
  for sample_time, sample_rate in table:
    print "// %s = %.02fKHz" % (sample_time, sample_rate)
    if min_rate and sample_rate >= min_rate:
      best_match = (sample_time, sample_rate)

  if min_rate:
    sample_time, sample_rate = best_match
    print "//"
    print "// Requested: %.2fKHz -> %.2fKHz" % (min_rate, sample_rate)
    print "#define ADC_PRESCALER %s" % (adc_prescaler)
    print "#define ADC_SAMPLE_TIME %s" % (sample_time)
    print "#define ADC_TWOSAMPLING_DELAY %s" % (two_sampling_delay)

if __name__ == '__main__':
  main(sys.argv)
