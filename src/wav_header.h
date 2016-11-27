/* wav _header.h defines a structure to hold the contents of a wav files header information */

struct WavHeader {
  //first 4 bytes of file, marks file as riff file. bytes 1-4
  unsigned char file_description_header[4];
  //next 4 bytes of file. bytes 5-8
  unsigned int file_size;
  //next 4 bytes file type header. Will always be 'WAVE' with wav file. bytes 9-12
  unsigned char file_type[4];

  //next 4 bytes format chunk marker, includes trailing null. bytes 13-16
  unsigned char format_description_header[4];
  //next 4 bytes length of format data listed above
  unsigned int format_data_length;

  //next 2 bytes
  unsigned int format_type;

  //next 2 bytes, mono ($01), stereo ($02)
  unsigned int num_channels;

  //next 4 bytes samples per second, frequency of quantization  (usually 44100Hz, 22050Hz, ..)
  unsigned int sample_rate;

  //next 4 bytes bytes per second - speed of data stream = num_channels * sample_rate * bits_per_sample (below) / 8
  unsigned int byte_rate;

  // next 2 bytes  num_cahnnels * bits_per_sample / 8 - number of bytes in elementary quantization
  unsigned int block_alignment;

  // next 2 bytes - digits of quantization
  unsigned int bits_per_sample;

  // next 4 bytes (usually ascii "data")
  unsigned char data_desc_header[4];

  // next 4 bytes (number of bytes of data)
  unsigned int data_size;

  //data follows header specified by data_size
};
