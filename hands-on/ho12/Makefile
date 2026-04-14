CC = gcc
CFLAGS = -Wall -Wextra -pthread

TARGETS = ex1_barrier ex2_prod_cons

all: $(TARGETS)

ex1_barrier: ex1_barrier.c
	$(CC) $(CFLAGS) -o $@ $<

ex2_prod_cons: ex2_prod_cons.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
