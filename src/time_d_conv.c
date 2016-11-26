#include <stdio.h>

//User defined header files
#include "wav_header.h"

int main(int argc, char*argv[])
{
  struct wav_header w_header;

  w_header.file_size = 200;
  printf("%d", w_header.file_size);

  return 0;
}
