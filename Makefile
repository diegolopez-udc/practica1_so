CC = gcc
CFLAGS = -g -Wall

p1: p1.c comandos.o files.o path.o listasimple.o
	$(CC) -o p1 $(CFLAGS) p1.c comandos.o files.o path.o listasimple.o

comandos.o: comandos.c comandos.h path.h
	$(CC) -c $(CFLAGS) comandos.c

path.o: path.c path.h listasimple.h
	$(CC) -c $(CFLAGS) path.c

listasimple.o: listasimple.c listasimple.h
	$(CC) -c $(CFLAGS) listasimple.c

files.o: files.c files.h
	$(CC) -c $(CFLAGS) files.c

clean:
	rm p1 *.o
