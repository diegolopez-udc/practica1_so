CC = gcc
CFLAGS = -g -Wall

p3: p3.c comandos.o files.o mem.o procesos.o path.o listasimple.o
	$(CC) -o p3 $(CFLAGS) p3.c comandos.o files.o mem.o procesos.o path.o listasimple.o

comandos.o: comandos.c comandos.h path.h
	$(CC) -c $(CFLAGS) comandos.c

path.o: path.c path.h listasimple.h
	$(CC) -c $(CFLAGS) path.c

listasimple.o: listasimple.c listasimple.h
	$(CC) -c $(CFLAGS) listasimple.c

files.o: files.c files.h
	$(CC) -c $(CFLAGS) files.c

mem.o: mem.c mem.h
	$(CC) -c $(CFLAGS) mem.c

procesos.o: procesos.c procesos.h
	$(CC) -c $(CFLAGS) procesos.c

clean:
	rm p3 *.o
