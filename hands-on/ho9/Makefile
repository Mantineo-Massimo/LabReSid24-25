CC = gcc
CFLAGS = -g -Wall
TARGET = programma
OBJS = main.o file1.o file2.o

all: $(TARGET) ex1_symbols

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

ex1_symbols: ex1_symbols.c
	$(CC) $(CFLAGS) -o ex1_symbols ex1_symbols.c

%.o: %.c
	$(CC) $(CFLAGS) -c $<

check:
	@echo "\n=== Simboli file1.o ==="
	nm file1.o
	@echo "\n=== Simboli file2.o ==="
	nm file2.o
	@echo "\n=== Simboli main.o ==="
	nm main.o
	@echo "\n=== Segmenti binario finale (programma) ==="
	size --format=GNU programma
	@echo "\n=== Segmenti binario base (ex1_symbols) ==="
	size --format=GNU ex1_symbols

clean:
	rm -f $(OBJS) $(TARGET) ex1_symbols *.o

.PHONY: all clean check
