CC = gcc
CFLAGS = -Wall -g -pthread

all: cs352proxy

check-syntax:
	$(CC) $(CFLAGS) -o /dev/null -S ${CHK_SOURCES}

cs352proxy: cs352proxy.c processconfig.o frames.o
	$(CC) $(CFLAGS) -o cs352proxy cs352proxy.c processconfig.o frames.o

processconfig.o: processconfig.c
	$(CC) $(CFLAGS) -c processconfig.c

frames.o: frames.c
	$(CC) $(CFLAGS) -c frames.c

