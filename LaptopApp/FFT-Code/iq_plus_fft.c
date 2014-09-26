/* FFT on IQ samples from the RTL_SDR (including fetching the bytes) 
 * Supposed to be a part of the Android NDK Application
 * for the end-to-end system.
 * 
 *
 * @author: Ayon Chakraborty
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "rtl-sdr.h"
#include "math.h"
                         
#define NFFT 512 
#define PI	M_PI	
#define TWOPI	(2.0*PI)
#define DEBUG 0

typedef struct FFT_data {
uint32_t freq;
double power;
} FFT_data_t;
FFT_data_t psd[10*NFFT+1];

int BUF_LEN = 2*NFFT;
int BUF_NUM = 1;
int NUM_SAMPLES = 100;
double iq_samples[2*NFFT+1];
static rtlsdr_dev_t *dev = NULL;

uint32_t start_frequency = 0;
uint32_t end_frequency = 0;
uint32_t current_center_frequency = 0;
uint32_t samp_rate = 0;
uint32_t resolution_bandwidth = 0;
uint32_t frequency_per_bin = 0;
float resolution_bandwidth_fraction = 0;
int bins_per_scan = 0;


int time_samples = 0;
int scans = 0;
int total_scans = 0;
int fft_rows = 0;

/* Function Declarations */
void FFT(double data[], int nn, int isign);
void process();
void flush_to_sdcard();
void rtlsdr_callback(unsigned char *buf, uint32_t len, void *ctx);
void *threadScan(void *arg);

int main(int argc, char *argv[]) {
	while(1) {
		if(argc != 3){
			printf("Usage: <exe> start_freq stop_freq res_bandwidth_frac\n");\
		}
		else {
			start_frequency = atoi(argv[1]);
			end_frequency =   atoi(argv[2]);
			resolution_bandwidth_fraction = 0.25; /*ratio to the sample rate */
			samp_rate = 1024000/4;
			current_center_frequency = start_frequency;
			resolution_bandwidth = resolution_bandwidth_fraction*samp_rate;
			bins_per_scan = resolution_bandwidth_fraction*NFFT;
			frequency_per_bin = samp_rate / NFFT;
			total_scans = (end_frequency - start_frequency) / resolution_bandwidth + 1 ;
			pthread_t pth1;
			pthread_create(&pth1,NULL,threadScan, 0);
			pthread_join(pth1,NULL);
		}
	}
}

void *threadScan(void* arg){

	uint32_t dev_index = 0;
	int gain = 0;
	int device_count,r=0,i=0;

	device_count = rtlsdr_get_device_count();
	rtlsdr_open(&dev, dev_index); // opening the device here
	r = rtlsdr_set_sample_rate(dev, samp_rate);
	r = rtlsdr_set_tuner_gain_mode(dev, 0);

	for(i=1; i<=total_scans; i++){
		printf("Issuing scan at %d, scan number = %d\n", current_center_frequency, i);
		r = rtlsdr_set_center_freq(dev, current_center_frequency);
		r = rtlsdr_reset_buffer(dev); 
		r = rtlsdr_read_async(dev, rtlsdr_callback, NULL, BUF_NUM, BUF_LEN);			
	}
	r = rtlsdr_close(dev);
	flush_to_sdcard();
	return 0;
}


void rtlsdr_callback(unsigned char *buf, uint32_t len, void *ctx) { 

	int i=0;
	double temp = 0;

	/* Normalizing */
	for(i=1; i<=BUF_LEN;  i=i+1){		
		temp = buf[i+1];
		temp /= (255/2);
		temp -- ;
		iq_samples[i] += temp;
	}

	time_samples = time_samples + 1;

	/* How many time samples you wan't to take and average upon? */
	if(time_samples == NUM_SAMPLES){
		for(i=1; i<=BUF_LEN;  i=i+1){		
			iq_samples[i] /= NUM_SAMPLES;
		}

		if(DEBUG) {
			for(i=1; i<=BUF_LEN;  i=i+2){		
				printf("(%f + j%f), ",iq_samples[i],iq_samples[i+1]);
			}
		}
		rtlsdr_cancel_async(dev);
		process();

		scans = scans + 1;
		current_center_frequency = current_center_frequency + resolution_bandwidth;
		time_samples = 0;
		for(i=1; i<=BUF_LEN;  i=i+1){		
			iq_samples[i] = 0;
		}
		return;	
	}
}


void process(){

	int i=0,j=0;
	int psd_start_bin, psd_end_bin;
	psd_start_bin = bins_per_scan * scans + 1;
	psd_end_bin = psd_start_bin + bins_per_scan - 1;	
	double power=0;
	//printf("(%d %d)\n ", psd_start_bin, psd_end_bin);

	/* Windowing, Hann. We will try Bartlett/Hamming also. */
	for (i=1; i<=BUF_LEN; i++) {
		double multiplier = 0.5 * (1 - cos(2*PI*i/(BUF_LEN-1)));
		iq_samples[i] = multiplier * iq_samples[i];
	}

	/* FFT, I am using 1, for zero it does the IFFT */
	FFT(iq_samples, NFFT, 1);

	/* Power Spectral Density */
	for(i=NFFT/2,j=psd_start_bin; i<NFFT/2+resolution_bandwidth_fraction*NFFT,j<=psd_end_bin; i++,j++)
	{
		power = 10*log10(sqrt(pow(iq_samples[2*i+1],2)+pow(iq_samples[2*i+2],2)));
		psd[j].power = power;
		psd[j].freq = start_frequency + (j-1)*frequency_per_bin;
		fft_rows += 1;
		//printf("%u %f \n",psd[j].freq, psd[j].power);	
	}	
}


void FFT(double data[], int nn, int isign) {
	int n, mmax, m, j, istep, i;
	double wtemp, wr, wpr, wpi, wi, theta;
	double tempr, tempi;

	n = nn << 1;
	j = 1;
	for (i = 1; i < n; i += 2) {
		if (j > i) {
			tempr = data[j];     data[j] = data[i];     data[i] = tempr;
			tempr = data[j+1]; data[j+1] = data[i+1]; data[i+1] = tempr;
		}
		m = n >> 1;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax = 2;
	while (n > mmax) {
		istep = 2*mmax;
		theta = TWOPI/(isign*mmax);
		wtemp = sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi = sin(theta);
		wr = 1.0;
		wi = 0.0;
		for (m = 1; m < mmax; m += 2) {
			for (i = m; i <= n; i += istep) {
				j =i + mmax;
				tempr = wr*data[j]   - wi*data[j+1];
				tempi = wr*data[j+1] + wi*data[j];
				data[j]   = data[i]   - tempr;
				data[j+1] = data[i+1] - tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wr = (wtemp = wr)*wpr - wi*wpi + wr;
			wi = wi*wpr + wtemp*wpi + wi;
		}
		mmax = istep;
	}
}


void flush_to_sdcard(){

	int i;
	FILE* fp = fopen("/home/ugupta/Documents/iq/dump.csv","wb+");
	fprintf(fp, "Frequency,Power\n");

	for(i=1; i<=fft_rows; i++){
		fprintf(fp, "%f,%f \n",psd[i].freq/1000000.0, psd[i].power);
	}

	fclose(fp);
}

