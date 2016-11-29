#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//User defined header files
#include "time_d_conv.h"
#include "typedefs.h"
#include "wav_header.h"

#define gotoxy(x,y) printf("\033[%d;%dH", (x), (y))

BOOL _Debug;

int main(int argc, char*argv[])
{
  struct WavHeader wav_header;
  struct WavHeader ir_header;
  FILE *wav_file,
    *ir_file;
  char *wav_file_str,
    *ir_file_str,
    *out_file_str;

  short *wav_data,
    *ir_data;

  float *fwav_data,
    *fir_data,
    *foutput;
  
  int wav_els,
    ir_els,
    out_els,
    fout_bytes;
  
  system("clear");
  
  printf("***********************************\n");
  printf("* ");
  printf("wav convolution processing tool");
  printf(" *\n");
  printf("***********************************\n\n");
  
  if(argc != 5){
    printf("Invalid number of arguments. Proper usage is %s %s %s %s %s\n",
	   argv[0],
	   "/dir/to/filename1.wav", "/dir/to/impulse_response.wav",
	   "out_filename.wav",
	   "[debug (true or false)]");
    return -1;
  }

  wav_file_str = argv[1];
  ir_file_str = argv[2];
  out_file_str = argv[3];
  
  if(strcmp(argv[4], "true") == 0)
    _Debug = TRUE;
  else
    _Debug = FALSE;

  printf("Running convolution processing with parameters:\nwav file: %s\nimpulse response file: %s\n\n",
	 wav_file_str,
	 ir_file_str);
  printf("Debug: %s\n", (_Debug == TRUE ? "true" : "false"));

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

    wav_data = getWavData(wav_file, wav_header.data_size);
    //done with file, have header and data
    fclose(wav_file);

    ir_data = getWavData(ir_file, ir_header.data_size);
    fclose(ir_file);

    printf("Wav samples read for input file and ir file.\n");

    //convert to float, divide by divisor 32768
    fwav_data = shortArrToFloat(wav_data, wav_header.data_size, SHORT_DIVISOR);
    fir_data = shortArrToFloat(ir_data, ir_header.data_size, SHORT_DIVISOR);
    
    if(_Debug == TRUE)
      displayTestInformation(wav_header, ir_header, wav_data, ir_data,
			     fwav_data, fir_data);
    
    wav_els = wav_header.data_size / BYTES_FLOAT;
    ir_els = ir_header.data_size / BYTES_FLOAT;
    out_els = wav_els + ir_els - 1;
   
    fout_bytes = out_els * BYTES_FLOAT;

    foutput = malloc(fout_bytes);

    printf("Allocating %d bytes for output samples.\n", fout_bytes);
    
    if(wav_data == NULL){
      printf("Error allocating memory for wav file data.\n");
      printf("Exitting...\n");
    }else if(ir_data == NULL){
      printf("Error allocating memory for impulse response file data.\n");
      printf("Exitting...\n");
    }else if(foutput == NULL){
      printf("Error allocating memory for output file data.\n");
      printf("Exitting...\n");
    }else{
      printf("Beginning convolution...\n");
      convolve(fwav_data,
	       wav_els,
	       fir_data,
	       ir_els,
	       foutput,
	       out_els);

      saveOutput(out_file_str, foutput, fout_bytes, wav_header);
      //saveOutput(foutput, out_els);
    }
  }

  printf("before cleanup\n");
  cleanup(wav_data, fwav_data, ir_data, fir_data);
   
  return 0;
}

void saveOutput(char *out_file_str, float *foutput, unsigned int fout_bytes,
		struct WavHeader wav_header){
  unsigned int out_bytes;
  short *output;
  FILE *fp;
  short sBuffer;
  int iBuffer;
  int i, remaining;
  
  output = floatArrToShort(foutput, &out_bytes, fout_bytes, SHORT_MULTIPLIER);
  if(_Debug == TRUE)
    printf("Float output bytes: %u\nShort output bytes: %u\nExpected output bytes: %u",
	   fout_bytes, out_bytes, fout_bytes / 2);
  
  fp = fopen(out_file_str ,"w+");
  
  fwrite("RIFF", 4, BYTE, fp);

  iBuffer = out_bytes - 36;
  fwrite(&iBuffer, BYTES_INT, BYTE, fp);
  fwrite("WAVE", 4, BYTE, fp);
  fwrite("fmt\0", 4, BYTE, fp);

  //data chunk is 16 bytes long
  iBuffer = 16;
  fwrite(&iBuffer, BYTES_INT, BYTE, fp);
  //same format as header
  fwrite(&wav_header.format_type, sizeof(wav_header.format_type), BYTE, fp);
  fwrite(&wav_header.num_channels, sizeof(wav_header.num_channels), BYTE, fp);
  fwrite(&wav_header.sample_rate, sizeof(wav_header.sample_rate), BYTE, fp);
  fwrite(&wav_header.byte_rate, sizeof(wav_header.byte_rate), BYTE, fp);
  fwrite(&wav_header.block_alignment, sizeof(wav_header.block_alignment), BYTE, fp);
  fwrite("data", 4, BYTE, fp);
  fwrite(output, out_bytes, BYTE, fp);
  /*
  printf("trying to write 1000 bytes of buffer\n");
  fwrite(output, 1000, BYTE, fp);
  */
  /*
  for(i = 0; i < out_bytes - 1000; i+=1000){
    printf("Printed bytes %d-%d\n", i, i+1000);
    fwrite(&output[i], 1000, BYTE, fp);
  }

  remaining = 1000 - (i - out_bytes);

  fwrite(&output[out_bytes - remaining], remaining, BYTE, fp);
  */
  fclose(fp);
}

/*
 * Function: intArrToFloat
 *
 * Description: takes in a 
 */
float* shortArrToFloat(short* arr, unsigned int size, float divisor){
  //short has 2 bytes, float has 4, must allocate 2x the space
  //Also ensure that float array is a multiple of 4
  float *toReturn = malloc((size + (size % BYTES_FLOAT)) * 2);
  int i;

  //size is in bytes, short is 2 bytes long - must divide by 2
  for(i = 0; i < (size / BYTES_SHORT); i++){
    toReturn[i] = (float)(arr[i] / divisor);
  }

  if(_Debug == FALSE){
    free(arr);
  }

  return toReturn;
}

short* floatArrToShort(float* arr, unsigned int *out_bytes, unsigned int size, float multiplier){
  int i;
  short *output;
  //ensure we are short aligned. Also need half the number of bytes for short
  (*out_bytes) = (size + size % BYTES_SHORT) / 2;
  output = malloc(*out_bytes);
  if(output != NULL){
    for (i = 0; i < (size / BYTES_FLOAT); i++){
      output[i] = (short)(arr[i] * multiplier);
    }

    if(_Debug == FALSE){
      free(arr);
    }
  }else{
    (*out_bytes) = 0;
    if(_Debug == TRUE)
      printf("Failed to allocate memory for short array in floatArrToShort\n");
  }
  
  return output;
}
/*
 * Function: getWavData
 * 
 * Description: 
 */
short* getWavData(FILE *fp, int data_size){
  short *outputBuffer = (short*)malloc(data_size);
  int i;
  if(fp != NULL && outputBuffer != NULL){
    //read data_size BYTES from fp into outputBuffer
    i = fread(outputBuffer, BYTE, data_size, fp);
  }
  
  if(_Debug == TRUE)
    printf("getWavData: %d bytes read. Expected %d bytes.\n", i, data_size);

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
  
  long finished = (long)w_size;
  double curPercent = 0;

  printf("w_size: %d, ir_size: %d, o_size: %d, finished_size: %llu\n", w_size, ir_size, o_size, finished);

  //save cursor position for progress
  printf("\033[s");
  fflush(stdout);
  
  if(o_size == (w_size + ir_size - 1)){
    for(out_index = 0; out_index < o_size; out_index++)
      output[out_index] = 0;
    
    for(wav_index = 0; wav_index < w_size; wav_index++){
      for(ir_index = 0; ir_index < ir_size; ir_index++){
	output[wav_index + ir_index] += wav_data[wav_index] * ir_data[ir_index];
      }

      curPercent = (double)((double)wav_index / (double)finished) * 100;
      printf("%.4f%%", curPercent);
      printf("\033[u");
      fflush(stdout);
    }

    printf("\n");
    
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


void cleanup(short *wav_data, float *fwav_data, short *ir_data, float *fir_data){
  //if we are not in debug mode, we freed wav_data and ir_data as soon as we were done with them
  //if we are in debug mode, we held off on freeing them to generate debug output
  if(!_Debug){
    free(wav_data);
    free(ir_data);
  }
  
  free(fwav_data);
  free(fir_data);
}



/****************************************************************************************

                                    TEST FUNCTIONS

 ****************************************************************************************/



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
void displayShortArrData(short *arr, int startEl, int numEls){
  int i;
  if (arr != NULL){
    printf("displayShortArray - Elements %d to %d:\n{ ", startEl, startEl + numEls);
    
    for(i = startEl; i < (startEl + numEls - 1); i++)
      printf("%d, ", arr[i]);
    
    printf("%d }\n", arr[i]);
  }else
    printf("Call to test display short array elements function failed. Passed in array was null.\n");
}

/*
 *
 */
void displayFloatArrData(float *arr, int startEl, int numEls){
  int i;

  if (arr != NULL){
    printf("displayFloatArray - Elements %d to %d:\n{ ", startEl, startEl + numEls);

    for(i = startEl; i < (startEl + numEls - 1); i++)
      printf("%f, ", arr[i]);
    
    printf("%f }\n", arr[i]);
  }else
    printf("Call to test display float array elements function failed. Passed in array was null.\n");
}

/*
 *
 */

short getMaxElement(short *arr, int numEls){
  int i;
  short max = -32768;
  for(i = 0; i < numEls; i++)
    if(arr[i] > max)
      max = arr[i];

  return max;
}

short getMinElement(short *arr, int numEls){
  int i;
  short min = 32767;
  for(i = 0; i < numEls; i++)
    if(arr[i] < min)
      min = arr[i];
  
  return min;
}


void displayTestInformation(struct WavHeader wav_header, struct WavHeader ir_header,
			    short *wav_data, short *ir_data,
			    float *fwav_data, float *fir_data){
    printf("Input wav file information:\n");
    displayHeaderInfo(wav_header);
    printf("\n");
    
    printf("Impulse response wav file information:\n");
    displayHeaderInfo(ir_header);
    
    printf("Input wav file sample data display\n");
    //divide byte size by 2 to get to short index, by 2 again to get to middle of array
    //print 20 elements
    displayShortArrData(wav_data, wav_header.data_size / 4, 15);
    printf("Max element in wav samples: %d\n", getMaxElement(wav_data, wav_header.data_size / 2));
    printf("Min element in wav sampels: %d\n", getMinElement(wav_data, wav_header.data_size / 2));
    printf("\n");

    printf("Input wav file sample data converted to float\n");
    displayFloatArrData(fwav_data, wav_header.data_size / 4, 15);
    printf("\n");
    
    printf("IR wav file sample data display\n");
    displayShortArrData(ir_data, ir_header.data_size / 4, 15);
    printf("Max element in ir samples: %d\n", getMaxElement(ir_data, ir_header.data_size / 2));
    printf("Min element in ir samples: %d\n", getMinElement(ir_data, ir_header.data_size / 2));
    printf("\n");
      
    printf("IR wav file sample data converted to float\n");
    displayFloatArrData(fir_data, ir_header.data_size / 4, 15);
    printf("\n");    
}
