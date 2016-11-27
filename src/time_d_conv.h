/* time_d_conv.h */

#ifndef TIME_D_CONV_H
#define TIME_D_CONV_H

struct WavHeader getHeaderInfo(FILE *fp);
void displayHeaderInfo(struct WavHeader header);
void displayArrayHeaderField(char *arr, int size, char *fieldName);
void displayArray(char *arr, int size);
char* getWavData(FILE *fp, int data_size);
int convolve(char *wav_data, int w_size, char *ir_data, int ir_size, char *output, int o_size);

#endif
