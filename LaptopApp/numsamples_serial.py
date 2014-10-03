
#/usr/bin/python

import serial
import time    

threshold = 10
start = time.time()

best_run = threshold
ser= serial.Serial('/dev/ttyACM0', 115200)
#ser= serial.Serial('/dev/ttyACM0', 9600)
samples=0

while samples < threshold: 
	print ser.readline()
	samples=samples+1

sample_rate=float(samples)/float(threshold)
print "Numer of Samples :%d" % samples, "Sample Rate :%f" % sample_rate 




