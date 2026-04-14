CC = gcc
CFLAGS = -Wall -Wextra -pthread

TARGETS = ex1_no_sync ex2_sync ex3_semaphore

all: $(TARGETS)

ex1_no_sync: ex1_no_sync.c
	$(CC) $(CFLAGS) -o $@ $<

ex2_sync: ex2_sync.c
	$(CC) $(CFLAGS) -o $@ $<

ex3_semaphore: ex3_semaphore.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
