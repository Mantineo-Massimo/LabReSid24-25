CC = gcc
CFLAGS = -Wall -Wextra

TARGETS = flooding

all: $(TARGETS)

flooding: flooding.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
	rm -f /tmp/fifo_*

.PHONY: all clean
