CC=gcc
TARGET=convolve
SDIR=./src
TIME_DIR=$(SDIR)/time_domain
FREQ_DIR=$(SDIR)/freq_domain
OBJ_DIR=./obj
SHARED_DIR=$(SDIR)/shared
FREQ_OBJS=convolve.o utils.o
ODIR=./obj
BDIR=./bin
LINK=-lm

FREQ_TARGET = $(BDIR)/$(TARGET)_freq
TIME_TARGET = $(BDIR)/$(TARGET)_time

all:
	@echo 'make all undefined. Please use:'
	@echo -e '\nmake time_d - compiles dependencies for time domain convolution.'
	@echo 'make gprof_time_d - time domain convolution with -pg specified to allow for gprof profiling'
	@echo -e 'make freq_d - compiles dependencies for frequency domain convolution.'
	@echo 'make gprof_freq_d - frequency domain convolution with -pg specified to allow for gprof profiling'

time_d:	$(TIME_DIR)/convolve.c $(TIME_DIR)/convolve.h
	$(CC) -o $(TIME_TARGET) $(TIME_DIR)/$(TARGET).c
	$(call printUsage,time)

gprof_time_d: $(TIME_DIR)/convolve.c $(TIME_DIR)/convolve.h
	$(CC) -pg -o $(TIME_TARGET) $(TIME_DIR)/$(TARGET).c
	@echo -e '\nCompiled with -pg, use gprof $(BDIR)/$(TARGET)_time'
	$(call printUsage,time)

convolve.o: $(FREQ_DIR)/convolve.c $(FREQ_DIR)/convolve.h $(FREQ_DIR)/utils.h
	$(CC) -c $(FREQ_DIR)/convolve.c

utils.o: $(FREQ_DIR)/utils.c $(FREQ_DIR)/utils.h
	$(CC) -c $(FREQ_DIR)/utils.c

freq_d: $(FREQ_OBJS)
	$(CC) -o $(FREQ_TARGET) $(FREQ_OBJS) $(LINK)
	rm -f *.o

clean:
	$(RM) $(BDIR)/$(TARGET)_time
	$(RM) $(BDIR)/$(TARGET)_freq

define printUsage
	@echo -e '$(BDIR)/$(TARGET)_$1 inFile.wav irFile.wav outFile.wav debug'
	@echo -e '\tinFile.wav = input file for convolution.'
	@echo -e '\tirFile.wav = impulse response file for convolution.'
	@echo -e '\toutFile.wav = name of desired output .wav file.'
	@echo -e '\tdebug = t for debug mode, f for regular mode (defaults to false on bad input).'
endef
