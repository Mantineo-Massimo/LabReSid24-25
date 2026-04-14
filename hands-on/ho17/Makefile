CC = gcc
CFLAGS = -Wall -Wextra

TARGETS = bellman_ford

all: $(TARGETS)

bellman_ford: bellman_ford.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
	rm -f /tmp/fifo_bf_*

.PHONY: all clean
