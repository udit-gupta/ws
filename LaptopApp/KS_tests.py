'''
Created on Jun 26, 2014

@author: wings
'''

from scipy.stats import ks_2samp
import numpy as np

import matplotlib.pyplot as plt

class KS_Tests:
    def histogram(self,signals1,signals2):

	#alpha1 = [x for x in range(len(signals1))]
	alpha2 = [x for x in range(len(signals2))]
	#pos1 = np.arange(len(alpha1))
	pos2 = np.arange(len(alpha2))

	width = 500.0     # gives histogram aspect to the bar diagram

	#ax1 = plt.axes()
	#ax1.set_xticks(pos1 + (width / 2))

	ax2 = plt.axes()
	ax2.set_xticks(pos2 + (width / 2))
	
	#ax1.set_xticklabels(alpha1)
	ax2.set_xticklabels(alpha2)

	#plt.bar(pos1, signals1, width, color='r')
	plt.bar(pos2, signals2, width, color='r')
	plt.show()

    def extract(self,path1,path2):
        signals1 = list()
        signals2 = list()
        alldata1 = open(path1, 'rb').readlines()
        alldata2 = open(path2, 'rb').readlines()
        for data1 in alldata1:
        	signals1.append(float(data1.strip().split("{")[2].split(":")[1].split(",")[0]))
	
	#print signals1
        
        for data2 in alldata2:
        	signals2.append(float(data2.strip().split("{")[2].split(":")[1].split(",")[0]))
        	#signals2.append(data2.strip())
	#print "signals2 .."
	#print signals2
        
        print ks_2samp(signals1, signals2)
        self.histogram(signals1,signals2);
	return 0
    
    
if __name__ == "__main__":
    path1 = "/home/ugupta/Documents/iq/data_bw_1024.txt"
    path2 = "/home/ugupta/Documents/iq/data_bw_1024_by_4.txt"
    
    ss = KS_Tests()
    ss.extract(path1,path2)








