/* convolve.h */

#ifndef CONVOLVE_H
#define CONVOLVE_H

#include "typedefs.h"

struct WavHeader getHeaderInfo(FILE *fp);
short* getWavData(FILE *fp, int data_size);
int convolve(float *wav_data, int w_size, float *ir_data, int ir_size, float *output, int o_size);
float* shortArrToFloat(short* arr, unsigned int size, float divisor);
short* floatArrToShort(float* arr, unsigned int *out_bytes, unsigned int size, float multiplier);
void write_little_endian(unsigned int out, int bytes, FILE *fp);
void saveOutput(char *out_file_str, float *data, unsigned int size, struct WavHeader wav_header);
void cleanup(short *wav_data, float *fwav_data, short *ir_data, float *fir_data);

/* test function definitions */
void displayTestInformation(struct WavHeader wav_header, struct WavHeader ir_header,
			    short *wav_data, short *ir_data, float *fwav_data, float *fir_data);
void displayHeaderInfo(struct WavHeader header);
void displayArrayHeaderField(char *arr, int size, char *fieldName);
void displayArray(char *arr, int size);
void displayShortArrData(short *arr, int startEl, int numEls);
void displayFloatArrData(float *arr, int startEl, int numEls);
short getMaxElement(short *arr, int numEls);
short getMinElement(short *arr, int numEls);
float getMaxElementFloat(float *arr, int numEls);
void normalizeArray(float *arr, int numEls, float max);


#endif
