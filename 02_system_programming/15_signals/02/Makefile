TARGETS = sigreceiver sigsender
CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS :=

.PHONY: all clean

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<
     
clean:
	rm -f $(TARGETS)
