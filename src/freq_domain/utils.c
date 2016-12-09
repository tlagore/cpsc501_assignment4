
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "../shared/typedefs.h"



/*
shortArrToDouble takes in a short array, 
a size of the array in BYTES, and whether the program is in debug mode or not.

the function will allocate enough space for a double array with the same number
of elements as the double array (size * 2) and copy the elements over to the
new array.
 */
double* shortArrToDouble(short* arr, unsigned int size, BOOL debug){
  //short has 2 bytes, double has 8, must allocated 8/2=  4x the number of bytes
  //Also ensure that the bytes allocated for double array are 8 aligned
  int extraBytes = BYTES_DOUBLE / BYTES_SHORT;
  int doublePad = size % BYTES_DOUBLE;
  int requiredBytes = (size + doublePad) * extraBytes;
  double *toReturn = malloc(requiredBytes);
  int i;

  //size is in bytes, short is 2 bytes long - must divide by 2
  for(i = 0; i < (size / BYTES_SHORT); i++){
    toReturn[i] = (double)arr[i];
  }

  if(debug == FALSE){
    free(arr);
  }

  return toReturn;
}

/*
doubleArrtoShort takes in a double array, a by reference integer to output the 
number of bytes allocated, a size of the array in BYTES, and whether the program
is in debug mode or not.

the function will allocate enough space for a short array with the same number
of elements as the double array (size / 4) and copy the elements over to the
new array.
 */
short* doubleArrToShort(double* arr, unsigned int *out_bytes, unsigned int size, BOOL debug){
  int i;
  short *output;
  //ensure we are short aligned. Also need half the number of bytes for short
  (*out_bytes) = (size + size % BYTES_SHORT) / 4;
  output = malloc(*out_bytes);
  if(output != NULL){
    for (i = 0; i < (size / BYTES_DOUBLE); i++){
      output[i] = (short)(arr[i]);
    }

    if(debug == FALSE){
      free(arr);
    }
  }else{
    (*out_bytes) = 0;
    if(debug == TRUE)
      printf("Failed to allocate memory for short array in floatArrToShort\n");
  }
  
  return output;
}
