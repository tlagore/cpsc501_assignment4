#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

//User defined header files
#include "convolve.h"
#include "../shared/typedefs.h"
#include "../shared/wav_header.h"
#include "utils.h"

#define PI 3.141592653589793
#define TWO_PI 2 * PI
#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr
#define NEAREST_POW2(a) (1 << ((int)log2(a) + 1))

BOOL _Debug;


/*
  convolve.c
  
  convolve takes in a nuber of arguments and produces a convolved wav_file
  by converting the signals to the frequency domain then multiplying these
  frequencies.

  The arguments are:
  argv[0] - program name
  argv[1] - input wav file for convolution
  argv[2] - input impulse response file for convolution
  argv[3] - name of desired output.wav file (existing files overwritten)
  argv[4] - debug (t)rue or (f)alse. Debug defaults to false on bad input

  currently this program does not work, for more details please see the 
  commenting on the function convolve().

  As the purpose of this assignment is profiling and optimization, I will
  run all profiling queries with the assumption that although this program
  is not producing the correct output - it is taking the same amount of time
  to do so that it would if it were producing correct results.
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
    *ir_data,
    *output;

  double *fwav_data,
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

    //convert to double
    fwav_data = shortArrToDouble(wav_data, wav_header.data_size, _Debug);
    fir_data = shortArrToDouble(ir_data, ir_header.data_size, _Debug);
    
    if(_Debug == TRUE)
      displayTestInformation(wav_header, ir_header, wav_data, ir_data,
			     fwav_data, fir_data);
    
    wav_els = wav_header.data_size / wav_header.block_alignment;
    ir_els = ir_header.data_size / ir_header.block_alignment;

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
      output = convolve(fwav_data,
	       wav_els,
	       fir_data,
	       ir_els,
	       &fout_bytes);

      saveOutput(out_file_str, output, fout_bytes, wav_header);
      //saveOutput(foutput, out_els);
    }
  }

  if(_Debug)
    printf("Cleaning up data, freeing memory.");
  cleanup(wav_data, fwav_data, ir_data, fir_data);
   
  return 0;
}

/*
  convolve

  convolve takes in an wav files data as well as an impulse response wav files data,along with the size of each data set (in elements) and attempts to convolve
  the two signals by the following process:
  
  the wav_data and ir_data are placed in new arrays. Their array size is determined  by the size of the required output (wav_size + ir_size - 1), multiplied by 2 
  (to account for imaginary numbers), then rounded to the nearest power of 2
  (required for the fft usage).

  Each of these arrays is then modified to have 0s every second element to make
  space for imaginary numbers produced by the FFT.

  Once both arrays have been run through the FFT function, their contents are
  multiplied together with complex multiplication.

  The result of this computation is then run through an inverse FFT and every
  element is divided by the size of the array.

  Finally, the array is normalized to have the largest element be the maximum
  size of a 16 bit integer (32767) and is copied over to a short array and 
  returned.

KNOWN BUGS:
This convolve method does not work. I believe that the logic is sound, but 
there is a bug somewhere (possibly multiple) that is causing the output
to be flawed.

However, as the point of the assignment is optimization - I am certain that 
the convolve algorithm with the FFT as its root is taking the same amount of 
time that it would be with correct results. As such, I will run profiling tests
on this new code with the assumption that although it may not be producing
the proper output, it is taking the same amount of time to produce erroneous output
and is valid for profile testing.
 */
short* convolve(double *wav_data, int w_size, double *ir_data, int ir_size, int *num_bytes)
{
  BOOL success = FALSE;
  int ir_out_size, wav_out_size;
  unsigned int w_size_fft;
  double *wav_freq_response, *ir_freq_response;
  short *output;
  int out_bytes;
  
  out_bytes = ir_size + w_size - 1;

  w_size_fft = NEAREST_POW2(out_bytes * 2);

  //make them both the same length for later multiplication
  wav_freq_response = makeFFTCompatible(wav_data, w_size, w_size_fft);
  ir_freq_response = makeFFTCompatible(ir_data, ir_size, w_size_fft);
  
  if(_Debug){
    printf("Array post processing sample: Data should be spread data[1], 0, data[2], 0...\n");
    displayDoubleArrData(wav_data, w_size / 4, 6);
    displayDoubleArrData(wav_freq_response, w_size / 2, 12);
  }
  
  if(!_Debug){
    free(wav_data);
    free(ir_data);
  }

  if(wav_freq_response != NULL && ir_freq_response != NULL){
    //1 for FFT
    //use w_size_fft, they are both the same size

    if(_Debug)
      printf("First FFT input wav\n");
    fft(wav_freq_response - 1, w_size_fft >> 1, 1);
    //postprocessData(wav_freq_response, w_size);
    
    if(_Debug)
      printf("Second FFT freq response\n");
    fft(ir_freq_response - 1, w_size_fft >> 1, 1);
    // postprocessData(ir_freq_response, w_size);

    if(_Debug)
      printf("Multiplying complex\n");
    //multiplyComplex stores results in first array argument   
    multiplyComplex(wav_freq_response, ir_freq_response, w_size_fft);

    if(_Debug)
      printf("Inverse FFT\n");

    //convert back to time domain
    fft(wav_freq_response - 1, w_size_fft >> 1, -1);
    //normalize, divide by n
    normalizeArray(wav_freq_response, w_size_fft, w_size_fft >> 1);
    //postprocessData(wav_freq_response, w_size_fft);

    displayDoubleArrData(wav_freq_response, w_size / 4, 10);

    double maxElement = getMaxElementDouble(wav_freq_response, w_size_fft);

    if(maxElement > 32767){
      normalizeArray(wav_freq_response, w_size_fft, maxElement / 32767);
    }

    postprocessData(wav_freq_response, w_size_fft);
    
    output = doubleArrToShort(wav_freq_response, num_bytes,
			      out_bytes * BYTES_DOUBLE,
			      _Debug);

    displayShortArrData(output, *num_bytes / 4, 10);
			      
  }else{
    printf("Error allocating data to convolve. Exitting...\n");
  }

  return output;
}

/*
  FFT From Numerical Recipes in C, p. 507-508.

  Float data types changed to double. Data is complex so the array size *must* be of size
  nn*2. It is assumed that nn is a power of 2.

  isign = 1 for FFT and -1 for inverse FFT

  Code assumes the array starts at index 1, not 0, so 1 must be subtracted from data pointer
  before calling.
*/

void fft(double data[], int nn, int isign)
{
  unsigned long n, mmax, m, j, istep, i;
  double wtemp, wr, wpr, wpi, wi, theta;
  double tempr, tempi;
  
  //bit shift nn, ie divide by 2
  n = nn << 1;
  j = 1;

  for(i = 1; i < n; i+=2){
    if(j > i){
      SWAP(data[j], data[i]);
      SWAP(data[j+1], data[i+1]);
    }
    m = nn;
    while(m >= 2 && j > m){
      j -= m;
      m >>= 1;
    }
    j += m;
  }

  mmax = 2;
  while(n > mmax){
    istep = mmax << 1;
    theta = isign * (TWO_PI / mmax);
    wtemp = sin(0.5 * theta);
    wpr = -2.0 * wtemp * wtemp;
    wpi = sin(theta);
    wr = 1.0;
    wi = 0.0;
    for(m = 1; m < mmax; m+=2){
      for(i = m; i <= n; i += istep){
	j = i + mmax;
	//real
	tempr = wr * data[j] - wi * data[j+1];
	//imaginary
	tempi = wr * data[j+1] + wi * data[j];
	data[j] = data[i+1] - tempr;
	data[j+1] = data[i+1] - tempi;

	data[i] += tempr;
	data[i+1] += tempi;
      }

      wr = (wtemp = wr) * wpr - wi * wpi + wr;
      wi = wi * wpr + wtemp * wpi + wi;
    }
    mmax = istep;
  }
}

/*
  normalizeArray takes in an array of data, the size of the array,
  and a divisor and divides every element of the array by this divisor.

 */
void normalizeArray(double *arr, unsigned int size, double divisor){
  unsigned int i;
  for(i = 0; i < size; i++){
    arr[i] /= divisor;
  }
}

/*
  multiplyComplex takes in two arrays assumed to hold complex numbers
  in the format [R, I, R, I] where the combination of the two represents
  the (R)eal and (I)maginary and multiplies the elements.

  multiplyComplex assumes that both arrays are of the same size and 
  stores the results of the computation in a.
 */
void multiplyComplex(double *a, double *b, int size){
  int i;

  for(i = 0; i < size - 1; i+=2){
    a[i] = (a[i] * b[i]) - (a[i+1] * b[i+1]);
    a[i+1] = (a[i+1] * b[i]) + (a[i] * b[i+1]);
  }
}

/*
  makeFFTCompatibe takes in an array of data, the length of the data
  and the desired size of the FFT compatible array. It then allocates
  an array with enough memory to hold this end size and then calls
  preprocessData to pad each element with space for an imaginary number.

  It is assumed that endSize is at least 2x the size of length, as 
  each element will have an associated imaginary number and preprocessData
  does not check to ensure this correctness.
 */
double *makeFFTCompatible(double *data, int length, int endSize){
  if(_Debug)
    printf("in makeFFTCompatible, allocating new space for array.\n");
  
  //calloc to 0 out allocated memory
  double *out = calloc(endSize, BYTES_DOUBLE);

  if(out != NULL){
  //shift array to make room for imaginary numbers
    preprocessData(out, data, length);
  }

  return out;
}

/*
  preprocessData takes in an array to store output, the input array,
  and the number of elements in the input array and stores the data 
  in the output array leaving space for imaginary numbers every 2nd element.

  It is assumed that the supplied output array is at least 2x the size of
  the input array. An example input/output of this function would be:
  
  dataIn = (1, 2, 3, 4)
  numEls = 4
  dataOut = (0, 0, 0, 0, 0, 0, 0, 0)

  after processing:
  dataOut = (1, 0, 2, 0, 3, 0, 4, 0)
 */
void preprocessData(double *dataOut, double *dataIn, int numEls){
  int i;

  if(_Debug)
    printf("in preprocessData, 0 separating data of array for imaginary numbers");
  
  for(i = 0; i < numEls; i+=2){
    dataOut[i] = dataIn[i/2];
    dataOut[i+1] = 0.0;
  }
}

/*
  postprocessData assumes that the arra passed in is in the form
  Real, Imaginary, Real, Imaginary, etc.

  As there is no imaginary number that can be written to output, 
  the array is condensed to not include these elements.
  The result of an array such as (1, 2i, 3, 4i)
  would be (1, 3, 0, 0)

 */
void postprocessData(double *dataOut, unsigned int numEls){
  int i, j;

  for(i = 0, j = 1; i < numEls / 2; i+=2){
    dataOut[i] = dataOut[(j-1)*2];
  }

  for(i = numEls / 2; i < numEls; i++){
    dataOut[i] = 0.0;
  }
}
/*
  saveOutput

  Description: saveOutput takes in a filename, a short array assumed to 
  contain the data portion of a wav file, the number of bytes in this
  data, and a WavHeader struct meant to describe the wav file being written.

  It creates a new file with name out_file_str and writes the header
  content of the wav file in proper order, followed by the data.

  the resulting file should be a playable wav file if no data was corrupted.
 */
void saveOutput(char *out_file_str, short *output, unsigned int out_bytes,
		struct WavHeader wav_header){
  FILE *fp;
  int iBuffer;
  int i;

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
 * Function: getHeaderInfo
 *
 * Description: getHeaderInfo takes in a file (assumed to be a wav file) and reads
 * the first 44 bytes into a header file struct.
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


void cleanup(short *wav_data, double *fwav_data, short *ir_data, double *fir_data){
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
void displayDoubleArrData(double *arr, int startEl, int numEls){
  int i;

  if (arr != NULL){
    printf("displayDoubleArray - Elements %d to %d:\n{ ", startEl, startEl + numEls);

    for(i = startEl; i < (startEl + numEls - 1); i++)
      printf("%f, ", arr[i]);
    
    printf("%f }\n", arr[i]);
  }else
    printf("Call to test display double array elements function failed. Passed in array was null.\n");
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

double getMaxElementDouble(double *arr, int numEls){
  int i;
  double max = -1000000;
  for(i = 0; i < numEls; i++)
    if(abs(arr[i]) > max)
      max = arr[i];
  
  return max;
}


void displayTestInformation(struct WavHeader wav_header, struct WavHeader ir_header,
			    short *wav_data, short *ir_data,
			    double *fwav_data, double *fir_data){
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

    printf("Input wav file sample data converted to double\n");
    displayDoubleArrData(fwav_data, wav_header.data_size / 4, 15);
    printf("\n");
    
    printf("IR wav file sample data display\n");
    displayShortArrData(ir_data, ir_header.data_size / 4, 15);
    printf("Max element in ir samples: %d\n", getMaxElement(ir_data, ir_header.data_size / 2));
    printf("Min element in ir samples: %d\n", getMinElement(ir_data, ir_header.data_size / 2));
    printf("\n");
      
    printf("IR wav file sample data converted to double\n");
    displayDoubleArrData(fir_data, ir_header.data_size / 4, 15);
    printf("\n");    
}
