/* time_d_conv.h */

#ifndef TIME_D_CONV_H
#define TIME_D_CONV_H

struct WavHeader getHeaderInfo(FILE *fp);
void displayHeaderInfo(struct WavHeader header);
void displayArrayHeaderField(char *arr, int size, char *fieldName);
void displayArray(char *arr, int size);
float* getWavData(FILE *fp, int data_size);
int convolve(float *wav_data, int w_size, float *ir_data, int ir_size, float *output, int o_size);

#endif
