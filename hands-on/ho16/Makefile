CC = gcc
CFLAGS = -Wall -Wextra

TARGETS = floodmax lcr

all: $(TARGETS)

floodmax: floodmax.c
	$(CC) $(CFLAGS) -o $@ $<

lcr: lcr.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
	rm -f /tmp/fifo_fm_*
	rm -f /tmp/fifo_lcr_*

.PHONY: all clean
