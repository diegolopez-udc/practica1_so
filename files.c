#include "files.h"



static LISTASIMPLE TablaFicheros;

// Función comparativa:
int CompararDescriptor(const void *p1, const void *p2) {
    return (const tFicheroAbierto*)p1->df - (const tFicheroAbierto*)p2->df;
}

// Conversión mode a texto:
static void ModoATexto(int mode, char *out) {
    int acc = mode & O_ACCMODE;                        // Hacemos un AND a nivel de bit con la máscara definida en la librería fcntl
    if (acc == O_RDONLY) strcpy(out, "O_RDONLY");
    else if (acc == O_WRONLY) strcpy(out, "O_WRONLY");
    else if (acc == O_RDWR) strcpy(out, "O_RDWR");
    else strcpy(out, "UNKNOWN");

    if (mode & O_CREAT) strcat(out, " | O_CREAT");
    if (mode & O_EXCL)  strcat(out, " | O_EXCL");
    if (mode & O_TRUNC) strcat(out, " | O_TRUNC");
    if (mode & O_APPEND) strcat(out, " | O_APPEND");
}



//***********************************/**********
/*************OPERACIONES************************/


// Inicializamos la tabla de ficheros con los valores estándar (0, 1, 2):
void InicializarTablaFicheros() {
    // ENTRADA ESTÁNDAR (0):
    tFicheroAbierto *f0 = malloc(sizeof(tFicheroAbierto));       // Asignamos un espacio de memoria al fichero.
    f0->df = 0;                                                  // Asignamos su descriptor de archivo.
    strcpy(f0->nombre, "entrada estándar");                      // Asignamos su nombre.
    f0->mode = O_RDONLY;                                         // Asignamos su modo.
    AniadirElemento(TablaFicheros, f0);                          // Lo añadimos a la tabla de ficheros.

    // SALIDA ESTÁNDAR (1):
    tFicheroAbierto *f1 = malloc(sizeof(tFicheroAbierto));
    f1->df = 1;
    strcpy(f1->nombre, "salida estándar");
    f1->mode = O_WRONLY;
    AniadirElemento(TablaFicheros, f1);

    // ERROR ESTÁNDAR (2):
    tFicheroAbierto *f2 = malloc(sizeof(tFicheroAbierto));
    f2->df = 2;
    strcpy(f2->nombre, "salida de error estándar");
    f2->mode = O_WRONLY;
    AniadirElemento(TablaFicheros, f2);
}

// Para añadir cualquier fichero a la tabla:
int AniadirFicheroAbierto(int df, const char *nombre, int mode){
    tFicheroAbierto *f = malloc(sizeof(tFicheroAbierto));
    if (f == NULL) return -1;

    f->df = df;
    strcpy(f->nombre, nombre);
    f->mode = mode;

    //Posibles errores:
    if (f->nombre == NULL) {
        free(f);
        return -1;
    }
    if (AniadirElemento(&TablaFicheros, f) == -1) {
        free(nuevo->nombre);
        free(nuevo);
        return -1;
    }

    return 0;
}

// Para eliminar cualquier fichero de la tabla:
int EliminarFicheroAbierto(int df){
    //Fichero auxiliar cuyo df es el buscado:
    tFicheroAbierto target;
    target.df = df;

    // Busca la posición del elemento en la lista:
    int p = BuscarElemento(TablaFicheros, &target, CompararDescriptor);
    if (p == -1) return -1; // Si no lo encuentra

    // Si lo encuentra:
    tFicheroAbierto *item = (tFicheroAbierto *)GetElementoAtPos(TablaFicheros, p);
    
    // Libera la memoria asignada dinámicamente:
    free(item->nombre);
    free(item);

    // Elimina de la lista:
    BorrarElementoAtPos(&TablaFicheros, p);
    return 0;
}

char * NombreFicheroDescriptor(int df){
    //Fichero auxiliar cuyo df es el buscado:
    tFicheroAbierto target;
    target.df = df;

    int p = BuscarElemento(TablaFicheros, &target, CompararDescriptor);
    if (p == -1) return NULL;  // Si no lo encuentra

    // Si lo encuentra:
    ItemFichero *item = (tFicheroAbierto *)GetElementoAtPos(TablaFicheros, p);
    return item->nombre;
}


void ListarFicherosAbiertos(){
    tPosicion p = GetPrimerElemento(TablaFicheros);
    char textoModo[128];

    while (p != NULL) {
        tFicheroAbierto *item = (tFicheroAbierto *)GetElementoAtPos(TablaFicheros, p);
        ModoATexto(item->mode, textoModo);
        printf("Descriptor: %d -> %s (%s)\n", item->df, item->nombre, textoModo);
        p = GetSiguienteElemento(TablaFicheros, p);
    }
}

