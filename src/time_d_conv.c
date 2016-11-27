#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//User defined header files
#include "time_d_conv.h"
#include "typedefs.h"
#include "wav_header.h"

int main(int argc, char*argv[])
{
  struct WavHeader wav_header;
  struct WavHeader ir_header;
  FILE *wav_file,
    *ir_file;
  char *wav_file_str,
    *ir_file_str;

  short *wav_data,
    *ir_data,
    *output;

  float *fwav_data,
    *fir_data,
    *foutput;
  
  unsigned char out_size;

  system("clear");
  
  printf("***********************************\n");
  printf("* ");
  printf("wav convolution processing tool");
  printf(" *\n");
  printf("***********************************\n\n");
  
  if(argc != 3){
    printf("Invalid number of arguments. Proper usage is %s %s %s\n", argv[0], "/dir/to/filename1.wav", "/dir/to/impulse_response.wav");
    return -1;
  }

  wav_file_str = argv[1];
  ir_file_str = argv[2];

  printf("Running convolution processing with parameters:\nwav file: %s\nimpulse response file: %s\n\n", wav_file_str, ir_file_str);

  wav_file = fopen(wav_file_str, "r");
  ir_file = fopen(ir_file_str, "r");

  if(wav_file == NULL){
    printf("Error opening wav file: %s\nExitting...\n", wav_file_str);
  }else if (ir_file == NULL){
    printf("Error opening impulse response file: %s\nExitting...\n", ir_file_str);
  }else{
    printf("Files opened successfully. Reading in header information for files.\n");
    wav_header = getHeaderInfo(wav_file);
    ir_header = getHeaderInfo(ir_file);

    printf("Wav file information:\n");
    displayHeaderInfo(wav_header);
    printf("\n");
    
    printf("Impulse response file information:\n");
    displayHeaderInfo(ir_header);

    //wav_data = getWavData(wav_file, wav_header.data_size);
    //printf("done getWavData\n");
    //displayIntArrData(wav_data, 10);
    //done with file, have header and data
    //fclose(wav_file);
    //printf("done close wav_file\n");
    
    //fwav_data = intArrToFloat(wav_data, wav_header.data_size, 32768, TRUE);    
    //displayFloatArrData(fwav_data, 10);

    //printf("done with intArrToFloat\n");
    
    ir_data = getWavData(ir_file, ir_header.data_size);
    printf("got ir data\n");

    if(ir_data != NULL)
      displayShortArrData(ir_data, 10);
    else
      printf("ir_data was null...\n");

    printf("after display ir_data\n");
    fclose(ir_file);
    fir_data = shortArrToFloat(ir_data, ir_header.data_size, 32768, TRUE);
    displayFloatArrData(fir_data, 10);
    
    out_size = wav_header.data_size + ir_header.data_size - 1;
    output = malloc(out_size);

    printf("out_size: %d\n", out_size);
    
    if(wav_data == NULL){
      printf("Error allocating memory for wav file data.\n");
      printf("Exitting...\n");
    }else if(ir_data == NULL){
      printf("Error allocating memory for impulse response file data.\n");
      printf("Exitting...\n");
    }else if(output == NULL){
      printf("Error allocating memory for output file data.\n");
      printf("Exitting...\n");
    }else{
      //convolve(fwav_data, wav_header.data_size, fir_data, ir_header.data_size, foutput, out_size);
    }
  }
   
  return 0;
}

/*
 * Function: intArrToFloat
 *
 * Description: takes in a 
 */
float* shortArrToFloat(short* arr, unsigned int size, float divisor, int doFree){
  //must allocated 2x the amount of space as floats are 4 bytes long
  float *toReturn = malloc(size * 2);
  int i;

  //size is in bytes, int is 2 bytes long - must divide by 2
  for(i = 0; i < (size / 4); i++){
    toReturn[i] = (float)(arr[i]) / divisor;
  }

  printf("arr copied\n");

  if(doFree == TRUE){
    free(arr);
  }

  return toReturn;
}

/*
 * Function: getWavData
 * 
 * Description: 
 */

short* getWavData(FILE *fp, int data_size){
  short *outputBuffer = (short*)malloc(data_size);

  if(fp != NULL && outputBuffer != NULL){
    //read data_size BYTES from fp into outputBuffer
    fread(outputBuffer, BYTE, data_size, fp);
  }

  return outputBuffer;
}

/*
 *  Function: convolve
 *
 *
 */
int convolve(float *wav_data, int w_size, float *ir_data, int ir_size, float *output, int o_size){
  int success = FALSE;
  int wav_index, ir_index, out_index;
  
  if(o_size == (w_size + ir_size - 1)){
    for(out_index = 0; out_index < o_size; out_index++)
      output[out_index] = 0;

    for(wav_index = 0; wav_index < w_size; wav_index++){
      for(ir_index = 0; ir_index < ir_size; ir_index++)
	output[wav_index + ir_index] += wav_data[wav_index] * ir_data[ir_index];

      //todo finish
    }
    
    success = TRUE;
  }else{
    printf("Convolve parameter output size is invalid. Should be input_size + impulse_response_size - 1.\n");
    printf("Expected %d but got %d\n", w_size + ir_size - 1, o_size);
    printf("Aborting convolution.\n");
    success = FALSE;
  }
  
  return success;
}

/*
 * Function: getHeaderInfo
 *
 * Description: getHeaderInfo takes in a file (assumed to be a wav file) and reads
 * the first 44 bytes into a header file struct.
 */
struct WavHeader getHeaderInfo(FILE *fp){
  struct WavHeader header;
  
  if(fp != NULL){
    //fread(&header, BYTE, HEADER_SIZE, fp);
    fread(&header.file_description_header, sizeof(header.file_description_header), BYTE, fp);
    fread(&header.file_size, sizeof(header.file_size), BYTE, fp);
    fread(&header.file_type,  sizeof(header.file_type), BYTE, fp);
    fread(&header.format_description_header, sizeof(header.format_description_header), BYTE, fp);
    fread(&header.format_data_length, sizeof(header.format_data_length), BYTE, fp);
    fread(&header.format_type, sizeof(header.format_type), BYTE, fp);
    fread(&header.num_channels, sizeof(header.num_channels), BYTE, fp);
    fread(&header.sample_rate, sizeof(header.sample_rate), BYTE, fp);
    fread(&header.byte_rate, sizeof(header.byte_rate), BYTE, fp);
    fread(&header.block_alignment, sizeof(header.block_alignment), BYTE, fp);
    fread(&header.bits_per_sample, sizeof(header.bits_per_sample), BYTE, fp);

    //MIN_DATA_LENGTH specifies the minimum chunk size of the data section of the wav header
    //if header.format_data_length is larger then this, there will be extra parameters

    if(header.format_data_length - MIN_DATA_LENGTH > 0){
      header.extra_params = malloc(header.format_data_length- MIN_DATA_LENGTH);
      fread(header.extra_params, (header.format_data_length - MIN_DATA_LENGTH), BYTE, fp);
    }	  

    fread(&header.data_desc_header, sizeof(header.data_desc_header), BYTE, fp);
    fread(&header.data_size, sizeof(header.data_size), BYTE, fp);
  }

  
  return header;
}



/*
 * displayHeaderInfo takes in a WavHeader struct and displays all the fields to screen
 *
 * as each char array is *not* null terminated to aid with memcpy uses, char arrays must
 *  printed explicitly.
 */
void displayHeaderInfo(struct WavHeader header){
  
  displayArrayHeaderField(header.file_description_header, 4, "File Description");
  printf("  File Size: %u Bytes\n", header.file_size);
  displayArrayHeaderField(header.file_type, 4, "  File Type");

  printf("\nSubchunk ");
  displayArray(header.format_description_header, 4);
  printf("\n");

  printf("  Format Data Length: %u Bytes\n", header.format_data_length);
  
  printf("  Audio Format: %u\n", header.format_type);
  printf("  Number of channels: %u\n", header.num_channels);
  printf("  Sample Rate: %u\n", header.sample_rate);
  printf("  Byte Rate: %u\n", header.byte_rate);
  printf("  Bits Per Sample: %u\n", header.bits_per_sample);
  

  printf("\n");
  printf("Subchunk ");
  displayArray(header.data_desc_header, 4);
  printf("\n");

  printf("  Data size: %u Bytes\n", header.data_size);
  
  return;
}

/*
 * Function: displayArrayHeaderField
 * 
 * Description: prints an arry field to screen with a specified header fieldName
 * 
 */
void displayArrayHeaderField(char *arr, int size, char *fieldName){
  printf("%s: ", fieldName);
  displayArray(arr, size);
  printf("\n"); 
}

/*
 * Function: displayArray 
 * 
 * Description: takes in a char array and a size and prints that number of characters of 
 * the array to screen
 */
void displayArray(char *arr, int size){
  int i;
  
  if(size <= sizeof(arr)){
    for(i = 0; i < size; i++)
      printf("%c", arr[i]);
  }
  
  return;
}

/*
 *
 */
void displayShortArrData(short *arr, int numEls){
  int i;

  printf("before loop\n");
  for(i = 0; i < numEls - 1; i++)
    printf("%d, ", arr[i]);

  printf("after loop\n");
  printf("%d\n", arr[i]);
}

/*
 *
 */
void displayFloatArrData(float *arr, int numEls){
  int i;

  printf("inside display float\n");
  
  for(i = 0; i < numEls - 1; i++)
    printf("%f, ", arr[i]);

  printf("%f\n", arr[i]);
}
