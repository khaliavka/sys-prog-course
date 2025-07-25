TARGET = program
CC = gcc
CFLAGS =
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
LIBS :=
MISC := $(wildcard *.txt)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $< $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
     
clean:
	rm -f $(OBJS) $(TARGET) $(MISC)
