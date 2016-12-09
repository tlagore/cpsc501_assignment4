/* convolve.h */

#ifndef CONVOLVE_H
#define CONVOLVE_H

#include "../shared/typedefs.h"

struct WavHeader getHeaderInfo(FILE *fp);
short* getWavData(FILE *fp, int data_size);
short* convolve(double *wav_data, int w_size, double *ir_data, int ir_size, int *numEls);
void write_little_endian(unsigned int out, int bytes, FILE *fp);
void saveOutput(char *out_file_str, short *data, unsigned int size, struct WavHeader wav_header);
void cleanup(short *wav_data, double *fwav_data, short *ir_data, double *fir_data);
void fft(double data[], int nn, int isign);
double *makeFFTCompatible(double *data, int length, int endSize);
void multiplyComplex(double *a, double *b, int size);
void normalizeArray(double *arr, unsigned int size, double divisor);
void preprocessData(double *dataOut, double *dataIn, int numEls);
void postprocessData(double *data, unsigned int numEls);

/* test function definitions */
void displayTestInformation(struct WavHeader wav_header, struct WavHeader ir_header,
			    short *wav_data, short *ir_data, double *fwav_data, double *fir_data);
void displayHeaderInfo(struct WavHeader header);
void displayArrayHeaderField(char *arr, int size, char *fieldName);
void displayArray(char *arr, int size);
void displayShortArrData(short *arr, int startEl, int numEls);
void displayDoubleArrData(double *arr, int startEl, int numEls);
short getMaxElement(short *arr, int numEls);
short getMinElement(short *arr, int numEls);
double getMaxElementDouble(double *arr, int numEls);


#endif
