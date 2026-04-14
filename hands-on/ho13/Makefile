CC = gcc
CFLAGS = -Wall -Wextra -pthread

TARGETS = ex1_join ex2_barrier

all: $(TARGETS)

ex1_join: ex1_join.c
	$(CC) $(CFLAGS) -o $@ $<

ex2_barrier: ex2_barrier.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
