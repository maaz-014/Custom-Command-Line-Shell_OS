CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -D_POSIX_C_SOURCE=200809L

all: myshell

myshell: main.o parser.o builtins.o signals.o redirect.o executor.o
	$(CC) $(CFLAGS) -o myshell main.o parser.o builtins.o signals.o redirect.o executor.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

parser.o: parser.c
	$(CC) $(CFLAGS) -c parser.c

builtins.o: builtins.c
	$(CC) $(CFLAGS) -c builtins.c

signals.o: signals.c
	$(CC) $(CFLAGS) -c signals.c

redirect.o: redirect.c
	$(CC) $(CFLAGS) -c redirect.c

executor.o: executor.c
	$(CC) $(CFLAGS) -c executor.c

clean:
	rm -f *.o myshell