CC = gcc
CFLAGS = -O2 -Wall
TARGET = ttcp.out

all: $(TARGET)

$(TARGET): ttcp.c
	$(CC) $(CFLAGS) -o $(TARGET) ttcp.c

clean:
	rm -f $(TARGET)
