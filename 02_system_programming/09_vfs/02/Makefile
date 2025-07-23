TARGET = program
CC = gcc
CFLAGS = 
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
     
clean:
	rm -f $(OBJS) $(TARGET)

