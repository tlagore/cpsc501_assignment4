CC=gcc
TARGET=time_d_conv

all: time_d_conv.c wav_header.h
	$(CC) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET)

