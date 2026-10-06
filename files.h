#include <fcntl.h>    // Manipular descriptores de ficheros.
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "listasimple.h"

/* Para compilar el .o: gcc -Wall -Wextra -c files.c -o files.o*/

// Definimos el tipo del fichero:
typedef struct {
    int df;             // Descriptor de fichero (p. ej. 3, 4, 5...)
    char nombre[1024];  // Nombre del fichero
    int mode;           // Modo de apertura
} tFicheroAbierto;

//OPERACIONES:
static void ModoATexto(int mode, char *out);
int TextoAModo(char *tr[], int mode);

void InicializarTablaFicheros();
int AniadirFicheroAbierto(int df, char *nombre, int mode);
int EliminarFicheroAbierto(int df);
char * NombreFicheroDescriptor(int df);
int ModoFicheroDescriptor(int df);
void ListarFicherosAbiertos();