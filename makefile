CC=gcc
TARGET=time_d_conv

all: time_d_conv.c
	$(CC) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET)
