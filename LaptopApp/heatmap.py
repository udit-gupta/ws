'''
Created on Jun 26, 2014

@author: wings
'''

from scipy.stats import ks_2samp
import numpy as np

import matplotlib.pyplot as plt
from os import listdir

class HeatMap:

    def plot(self,path):

	twodlist=list()
	cnt=0
	
	filelist=list()
	for i in range(10):
		for j in range(20):
			filelist.append(str(i+1)+''+str(j+1)+'.txt')
	#print filelist
	
	
	for files in filelist:
		#print files
        	alldata1 = open(path+files, 'rb').readlines()
		alldata=list()
		for i in alldata1:
			if i!='\n':
				alldata.append(i.rstrip('\n'))
		medianarray=list()
		for data in alldata[:-1]:
			medianarray.append(float(data.strip().split("{")[2].split(":")[1].split(",")[0]))
		#print medianarray			
		a=np.array(medianarray)
		current=np.median(a)	
		#exit(-1)
		twodlist.append(current)
		#exit(-1)

	#print twodlist
	
	list1=list()
	cnt=0
	for i in range(10):
		list2=list()
		for j in range(20):
			list2.append(twodlist[cnt])
			cnt=cnt+1
		list1.append(list2)
	
        thefile = open(path+'output.txt', 'w')
	
	thefile.write("INDEX, G1,G2,G3,G4,G5,G6,G7,G8,G9,G10,G11,G12,G13,G14,G15,G16,G17,G18,G19,G20")
	thefile.write("\n")
	i=0
	for item in list1:
		i=i+1
		thefile.write("Sample %d" %i)
		for item2 in item:
			thefile.write(",%s" % item2)
		thefile.write("\n")
		

	'''
	for k in list1:
		print k
		print '\n'
	'''

	return 0
    
    
if __name__ == "__main__":
    path1 = "/home/ugupta/Documents/iq/logs/"
    
    ss = HeatMap()
    ss.plot(path1)








