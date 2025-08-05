CFLAGS= -g -Wall -Wextra

all: 1compile

1compile: 1compile.c, tokens.o, parse.o, transfer.o

parse.o: tokens.o

clean:
	rm -f *.o 1compile
