/* time_d_conv.h */

#ifndef TIME_D_CONV_H
#define TIME_D_CONV_H

struct WavHeader getHeaderInfo(FILE *fp);
void displayHeaderInfo(struct WavHeader header);
void displayArrayHeaderField(char *arr, int size, char *fieldName);
void displayArray(char *arr, int size);
short* getWavData(FILE *fp, int data_size);
int convolve(float *wav_data, int w_size, float *ir_data, int ir_size, float *output, int o_size);
float* shortArrToFloat(short* arr, unsigned int size, float divisor, int free);
void displayShortArrData(short *arr, int numEls);
void displayFloatArrData(float *arr, int numEls);


#endif
