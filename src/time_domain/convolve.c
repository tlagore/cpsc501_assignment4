#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//User defined header files
#include "convolve.h"
#include "../shared/typedefs.h"
#include "../shared/wav_header.h"

BOOL _Debug;

/*
  convolve.c
  
  convolve takes in a nuber of arguments and produces a convolved wav_file
  convolve calculates the convolved signal in the time domain and therefore
  takes a very long time to run.

  The arguments are:
  argv[0] - program name
  argv[1] - input wav file for convolution
  argv[2] - input impulse response file for convolution
  argv[3] - name of desired output.wav file (existing files overwritten)
  argv[4] - debug (t)rue or (f)alse. Debug defaults to false on bad input
 */
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
    printf("Invalid number of arguments. Proper usage is %s inFile irFile outFile debug\n", argv[0]);
    printf("where\n");
    printf("\tinFile = /dir/to/filename1.wav");
    printf("\tirFile = /dir/to/impulse_response.wav");
    printf("\toutFile = out_filename.wav");
    printf("\tdebug = [t]rue or [f]alse (defaults to false if malformed)");
    return -1; 
  }

  wav_file_str = argv[1];
  ir_file_str = argv[2];
  out_file_str = argv[3];
  
  if(strcmp(argv[4], "true") == 0 || strcmp(argv[4], "t") == 0)
    _Debug = TRUE;
  else
    _Debug = FALSE;

  
  if(access(out_file_str, F_OK) != -1){
    //file exists, if this detection is required later, insert handling code here
  }
  
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
    
    wav_els = wav_header.data_size / wav_header.block_alignment;
    ir_els = ir_header.data_size / ir_header.block_alignment;
    out_els = wav_els + ir_els - 1;
   
    fout_bytes = out_els * BYTES_FLOAT;

    foutput = malloc(fout_bytes);

    if(_Debug)
      printf("Allocating %d bytes for float output samples.\n", fout_bytes);
    
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

  if(_Debug)
    printf("Cleaning up data, freeing memory.");
  cleanup(wav_data, fwav_data, ir_data, fir_data);
   
  return 0;
}

/*
  saveOutput

  Description: saveOutput takes in a filename, a float array assumed to 
  contain the data portion of a wav file, the number of bytes in this
  data, and a WavHeader struct meant to describe the wav file being written.

  saveOutput first normalizes the array to ensure that no element exceeeds
  + or - 1.0. Then it converts the data back to a 16 bit integer array.

  It creates a new file with name out_file_str and writes the header
  content of the wav file in proper order, followed by the data.

  the resulting file should be a playable wav file if no data was corrupted.
 */
void saveOutput(char *out_file_str, float *foutput, unsigned int fout_bytes,
		struct WavHeader wav_header){
  unsigned int out_bytes;
  short *output;
  FILE *fp;
  int iBuffer;
  int i;
  float max;

  max = getMaxElementFloat(foutput, fout_bytes / 4);
  if (max > 1){
    if(_Debug)
      printf("Max element in output array before conversion was %f, normalizing..\n", max);
    
    normalizeArray(foutput, fout_bytes / 4, max);
  }

  output = floatArrToShort(foutput, &out_bytes, fout_bytes, SHORT_MULTIPLIER);
  if(_Debug == TRUE)
    printf("Float output bytes: %u\nShort output bytes: %u\nExpected output bytes: %u\n",
	   fout_bytes, out_bytes, fout_bytes / 2);
  
  fp = fopen(out_file_str ,"w+");
  
  fwrite("RIFF", 4, BYTE, fp);
  iBuffer = out_bytes - 36;
  write_little_endian(iBuffer, BYTES_INT, fp);
  //fwrite(&iBuffer, BYTES_INT, BYTE, fp);
  
  fwrite("WAVE", 4, BYTE, fp);
  fwrite("fmt ", 4, BYTE, fp);

  //data chunk is 16 bytes long
  iBuffer = 16;
  fwrite(&iBuffer, BYTES_INT, BYTE, fp);
  //same format as header
  write_little_endian(wav_header.format_type, sizeof(wav_header.format_type), fp);
  write_little_endian(wav_header.num_channels, sizeof(wav_header.num_channels), fp);
  write_little_endian(wav_header.sample_rate, sizeof(wav_header.sample_rate), fp);
  write_little_endian(wav_header.byte_rate, sizeof(wav_header.byte_rate), fp);
  write_little_endian(wav_header.block_alignment, sizeof(wav_header.block_alignment), fp);
  write_little_endian(wav_header.bits_per_sample, sizeof(wav_header.bits_per_sample), fp);
     
  fwrite("data", 4, BYTE, fp);
  write_little_endian(out_bytes, BYTES_INT, fp);

  if(_Debug == TRUE)
    printf("Header data printed to %s\n", out_file_str);

  printf("outbytes before write %u\n", out_bytes);
  
  for(i = 0; i < out_bytes / 2; i++)
    write_little_endian((unsigned int)(output[i]), sizeof(wav_header.block_alignment), fp);

  if(_Debug == TRUE)
    printf("Convoluted sample data written to file.\n");

  free(output);
  fclose(fp);
}

/*
  write_little_endian takes in an unsigned integer that needs to be 
  written as little endian and writes the bytes of the output to
  the little endian specification.
 */
void write_little_endian(unsigned int output, int bytes, FILE *fp)
{
    unsigned ch;
    while(bytes > 0)
    {
        ch = output & 0xff;
        fwrite(&ch, 1, 1, fp);
        bytes--;
	output >>= 8;
    }
}


/*
 * Function: intArrToFloat
 takes in a short array, a size, and a divisor, and returns a float 
 array with the elements of the short array divided by the divisor.
 
 To get the same elements, but in float form - set divisor = 1
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

/*
  

 */
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
 * Description: getWavData reads in the wav data portion of the wav file.
 * getWavData assumes that the fp passed in was used to read the header
 * file for the wav file up to the point where the wav data is located.
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
 *  convolve takes in input wav file data, an input wav file impulse response 
 * data, the size of each array, and an output array for storing the results
 * (along with the size of the output array).
 *
 * If the output is the appropriate size for the inputs, then convolve will 
 * convolve the two signals in the time domain. As this algorithm is n^2, 
 * convolve takes a long time to run (upwards of 30-45 minutes for any wav_file
 * over 2-3 minutes).
 */
int convolve(float *wav_data, int w_size, float *ir_data, int ir_size, float *output, int o_size){
  int success = FALSE;
  int wav_index, ir_index, out_index;
  
  long finished = (long)w_size;
  double curPercent = 0;

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
      printf(" %.2f%%", curPercent);
      printf("\033[u");
      fflush(stdout);
    }
    
    printf("100.00%%\n");
    
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
 * the first 44+ bytes into a header file struct.
 */
struct WavHeader getHeaderInfo(FILE *fp){
  struct WavHeader header;
  
  if(fp != NULL){
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
  cleanup frees any memory that has not been used.

  If the program is in debug mode, certain arrays were not freed to allow for
  debug information. If we are not in debug mode, then the arrays have already
  been freed.

 */
void cleanup(short *wav_data, float *fwav_data, short *ir_data, float *fir_data){
  //if we are not in debug mode, we freed wav_data and ir_data as soon as we were done with them
  //if we are in debug mode, we held off on freeing them to generate debug output
  if(_Debug){
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
 * displayShortArrData displays the elements of a short array from startEl
 * to startEl + numEls.
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
 * displayFloatArrData displays the elements of a float array from startEl
 * to startEl + numEls.
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
 *gets the ABSOLUTE max element of a short array
 */
short getMaxElement(short *arr, int numEls){
  int i;
  short max = -32768;
  for(i = 0; i < numEls; i++)
    if(abs(arr[i]) > max)
      max = abs(arr[i]);

  return max;
}

/*
  gets the minimum element of a short array

 */
short getMinElement(short *arr, int numEls){
  int i;
  short min = 32767;
  for(i = 0; i < numEls; i++)
    if(arr[i] < min)
      min = arr[i];
  
  return min;
}
/*
  gets the ABSOLUTE max element of a float array

 */
float getMaxElementFloat(float *arr, int numEls){
  int i;
  float max = -1000000;
  for(i = 0; i < numEls; i++)
    if(abs(arr[i]) > max)
      max = abs(arr[i]);
  
  return max;
}

void normalizeArray(float *arr, int numEls, float max){
  int i;
  for(i = 0; i < numEls; i++)
    arr[i] /= max;
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
