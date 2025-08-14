TARGET = server
CC = gcc
CFLAGS = -Wall -g
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
LIBS :=

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
     
clean:
	rm -f $(OBJS) $(TARGET)
