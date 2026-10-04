#include <fcntl.h>    // Manipular descriptores de ficheros.
#include <string.h>

#include "listasimple.h"

// Definimos el tipo del fichero:
typedef struct {
    int df;             // Descriptor de fichero (p. ej. 3, 4, 5...)
    char nombre[1024];  // Nombre del fichero
    int mode;           // Modo de apertura
} tFicheroAbierto;

//OPERACIONES:
void InicializarTablaFicheros();
int AniadirFicheroAbierto(int df, const char *nombre, int mode);
int EliminarFicheroAbierto(int df);
char * NombreFicheroDescriptor(int df);
void ListarFicherosAbiertos();