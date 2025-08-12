TARGETS = client server
CC = gcc
CFLAGS = -Wall
LIBS :=

.PHONY: all clean

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<
     
clean:
	rm -f $(TARGETS)
