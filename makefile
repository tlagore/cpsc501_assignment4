CC=gcc
TARGET=convolve
SDIR=src
ODIR=obj
BDIR=bin

all: ./$(SDIR)/convolve.c ./$(SDIR)/convolve.h ./$(SDIR)/wav_header.h
	$(CC) -o ./$(BDIR)/$(TARGET) ./$(SDIR)/$(TARGET).c
	@echo -e '\nUsage is:'
	$(call printUsage)

timed: ./$(SDIR)/convolve.c ./$(SDIR)/convolve.h ./$(SDIR)/wav_header.h
	$(CC) -pg -o ./$(BDIR)/$(TARGET) ./$(SDIR)/$(TARGET).c
	@echo -e '\nCompiled with -pg, use gprof ./$(BDIR)/$(TARGET)'
	$(call printUsage)

clean:
	$(RM) $(BDIR)/$(TARGET)


define printUsage
	@echo -e './$(BDIR)/$(TARGET) inFile.wav irFile.wav outFile.wav debug'
	@echo -e '\tinFile.wav = input file for convolution.'
	@echo -e '\tirFile.wav = impulse response file for convolution.'
	@echo -e '\toutFile.wav = name of desired output .wav file.'
	@echo -e '\tdebug = t for debug mode, f for regular mode (defaults to false on bad input).'
endef
