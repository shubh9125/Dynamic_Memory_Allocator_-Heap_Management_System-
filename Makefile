CC=gcc
CFLAGS=-Wall -g
TARGET=allocator

all: $(TARGET)

$(TARGET): final.c
	$(CC) $(CFLAGS) final.c -o $(TARGET)

clean:
	rm -f $(TARGET)
