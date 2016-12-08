/* utils.h */

#ifndef UTILS_H
#define UTILS_H

#include "../shared/typedefs.h"

double* shortArrToDouble(short* arr, unsigned int size, BOOL debug);
short* doubleArrToShort(double* arr, unsigned int *out_bytes, unsigned int size, BOOL debug);

#endif
