#include <stdio.h>
#include <stdlib.h>

//User defined header files
#include "wav_header.h"

struct WavHeader getHeaderInfo(FILE *fp);

int main(int argc, char*argv[])
{
  struct WavHeader wav_header;
  struct WavHeader ir_header;
  FILE *wav_file,
    *ir_file;
  char *wav_file_str,
    *ir_file_str;

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
    printf("%d\n", wav_header.file_size);
  }
  
  
  return 0;
}

struct WavHeader getHeaderInfo(FILE *fp){
  struct WavHeader header;
  
  
  if(fp != NULL){
    printf("we remembered how to call a function!\n");
    header.file_size = 200;
    return header;
  }
}
