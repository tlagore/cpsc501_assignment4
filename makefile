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
FREQ_TARGET_GPROF = $(BDIR)/$(TARGET)_freq_gp
TIME_TARGET = $(BDIR)/$(TARGET)_time
TIME_TARGET_GPROF = $(BDIR)/$(TARGET)_time_gp

all:
	make time_d
	make freq_d

time_d:	$(TIME_DIR)/convolve.c $(TIME_DIR)/convolve.h
	$(CC) -o $(TIME_TARGET) $(TIME_DIR)/$(TARGET).c $(opt)
	$(call printUsage,time)

time_d_gprof: $(TIME_DIR)/convolve.c $(TIME_DIR)/convolve.h
	$(CC) -pg -o $(TIME_TARGET_GPROF) $(TIME_DIR)/$(TARGET).c
	@echo 'Optimization set to $(opt)'
	@echo -e '\nCompiled with -pg, use $(TIME_TARGET_GPROF), then gprof $(TIME_TARGET_GPROF) gmon.out > out.txt'
	$(call printUsage,time_gp)

convolve.o: $(FREQ_DIR)/convolve.c $(FREQ_DIR)/convolve.h $(FREQ_DIR)/utils.h
	$(CC) -c $(FREQ_DIR)/convolve.c

utils.o: $(FREQ_DIR)/utils.c $(FREQ_DIR)/utils.h
	$(CC) -c $(FREQ_DIR)/utils.c

freq_d: $(FREQ_OBJS)
	$(CC) -o $(FREQ_TARGET) $(FREQ_OBJS) $(LINK)
	rm -f *.o
	$(call printUsage,freq)

freq_d_gprof: $(FREQ_OBJS)
	$(CC) $(opt) -pg -o $(FREQ_TARGET_GPROF) $(FREQ_OBJS) $(LINK)
	@echo 'Optimization set to $(opt) (if not set use make opt=-o1,-o2,-o3 target)'
	@echo -e '\nCompiled with -pg, use $(FREQ_TARGET_GPROF), then gprof $(FREQ_TARGET_GPROF) gmon.out > out.txt'
	$(call printUsage,freq_gp)

clean:
	$(RM) $(FREQ_TARGET)
	$(RM) $(TIME_TARGET)
	$(RM) $(FREQ_TARGET_GPROF)
	$(RM) $(TIME_TARGET_GPROF)

define printUsage
	@echo -e '$(BDIR)/$(TARGET)_$1 inFile.wav irFile.wav outFile.wav debug'
	@echo -e '\tinFile.wav = input file for convolution.'
	@echo -e '\tirFile.wav = impulse response file for convolution.'
	@echo -e '\toutFile.wav = name of desired output .wav file.'
	@echo -e '\tdebug = t for debug mode, f for regular mode (defaults to false on bad input).'
endef
