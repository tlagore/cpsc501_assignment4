#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//User defined header files
#include "wav_header.h"

#define BYTE 1

struct WavHeader getHeaderInfo(FILE *fp);
void displayHeaderInfo(struct WavHeader header);
void displayArrayHeaderField(char *arr, int size, char *fieldName);
void displayArray(char *arr, int size);

int main(int argc, char*argv[])
{
  struct WavHeader wav_header;
  struct WavHeader ir_header;
  FILE *wav_file,
    *ir_file;
  char *wav_file_str,
    *ir_file_str;

  char *wav_data;
  char *ir_data;
  
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

    wav_data = malloc(wav_header.data_size);
  }
  
  
  return 0;
}

struct WavHeader getHeaderInfo(FILE *fp){
  struct WavHeader header;
  char header_buffer[HEADER_SIZE];
  int i;
  int offset = 0;
  
  if(fp != NULL){
    fread(&header, BYTE, HEADER_SIZE, fp);
  }
  
  return header;
}



/**
   displayHeaderInfo takes in a WavHeader struct and displays all the fields to screen

   as each char array is *not* null terminated to aid with memcpy uses, char arrays must be printed
   explicitly.
 */
void displayHeaderInfo(struct WavHeader header){
  
  displayArrayHeaderField(header.file_description_header, 4, "File Description");
  printf("  File Size: %d Bytes\n", header.file_size);
  displayArrayHeaderField(header.file_type, 4, "  File Type");

  printf("\nSubchunk ");
  displayArray(header.format_description_header, 4);
  printf("\n");
  
  printf("  Audio Format: %d\n", header.format_type);
  printf("  Number of channels: %d\n", header.num_channels);
  printf("  Sample Rate: %d\n", header.sample_rate);
  printf("  Byte Rate: %d\n", header.byte_rate);
  printf("  Bits Per Sample: %d\n", header.bits_per_sample);

  printf("\n");
  printf("Subchunk ");
  displayArray(header.data_desc_header, 4);
  printf("\n");

  printf("  Data size: %d Bytes\n", header.data_size);
  
  return;
}


void displayArrayHeaderField(char *arr, int size, char *fieldName){
  printf("%s: ", fieldName);
  displayArray(arr, size);
  printf("\n"); 
}
/**
   displayArray takes in a char array and a size and prints that number of characters of the array to screen

 */
void displayArray(char *arr, int size){
  int i;
  
  if(size <= sizeof(arr)){
    for(i = 0; i < size; i++)
      printf("%c", arr[i]);
  }
  
  return;
}
