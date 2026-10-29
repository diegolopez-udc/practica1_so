CC = gcc
CFLAGS = -g -Wall

p2: p2.c comandos.o files.o mem.o path.o listasimple.o
	$(CC) -o p2 $(CFLAGS) p2.c comandos.o files.o mem.o path.o listasimple.o

comandos.o: comandos.c comandos.h path.h mem.h files.h listasimple.h
	$(CC) -c $(CFLAGS) comandos.c

path.o: path.c path.h listasimple.h
	$(CC) -c $(CFLAGS) path.c

listasimple.o: listasimple.c listasimple.h
	$(CC) -c $(CFLAGS) listasimple.c

files.o: files.c files.h
	$(CC) -c $(CFLAGS) files.c

mem.o: mem.c mem.h files.h listasimple.h
	$(CC) -c $(CFLAGS) mem.c

clean:
	rm -f p2 *.o
