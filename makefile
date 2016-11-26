CC=gcc
TARGET=time_d_conv
SDIR=src
ODIR=obj
BDIR=bin

all: ./$(SDIR)/time_d_conv.c ./$(SDIR)/wav_header.h
	$(CC) -o ./$(BDIR)/$(TARGET) ./$(SDIR)/$(TARGET).c

clean:
	$(RM) $(BDIR)/$(TARGET)
