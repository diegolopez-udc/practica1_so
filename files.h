#ifndef FILES_H
#define FILES_H

#include <fcntl.h>    // Manipular descriptores de ficheros.
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "listasimple.h"


// Definimos el tipo del fichero:
typedef struct {
    int df;             // Descriptor de fichero (p. ej. 3, 4, 5...)
    char nombre[1024];  // Nombre del fichero
    int mode;           // Modo de apertura
} tFicheroAbierto;

//OPERACIONES:
void ModoATexto(int mode, char *out);
int TextoAModo(char *tr[], int mode);

void InicializarTablaFicheros();
int AniadirFicheroAbierto(int df, char *nombre, int mode);
int EliminarFicheroAbierto(int df);
void VaciarTablaFicherosAbiertos();
char * NombreFicheroDescriptor(int df);
int ModoFicheroDescriptor(int df);
void ListarFicherosAbiertos();

#endif
