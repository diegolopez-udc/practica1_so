#include "files.h"



static LISTASIMPLE TablaFicheros;    // Aprovechamos listasimple para implementar la tabla de ficheros abiertos

// Función comparativa:
int CompararDescriptor(void *p1, void *p2) {
    const tFicheroAbierto *f1 = (const tFicheroAbierto *)p1;
    const tFicheroAbierto *f2 = (const tFicheroAbierto *)p2;
    return f1->df - f2->df; 
}

// Conversión mode a texto:
static void ModoATexto(int mode, char *out) {
    int acc = mode & O_ACCMODE;                        // Hacemos un AND a nivel de bit con la máscara definida en la librería fcntl
    if (acc == O_RDONLY) strcpy(out, "O_RDONLY");
    else if (acc == O_WRONLY) strcpy(out, "O_WRONLY");
    else if (acc == O_RDWR) strcpy(out, "O_RDWR");
    else strcpy(out, "UNKNOWN");

    if (mode & O_CREAT) strcat(out, " O_CREAT");
    if (mode & O_EXCL)  strcat(out, " O_EXCL");
    if (mode & O_TRUNC) strcat(out, " O_TRUNC");
    if (mode & O_APPEND) strcat(out, " O_APPEND");
}

// Conversión texto a modo:
int TextoAModo(char *tr[], int mode){
    int i;

    for (i=0; tr[i] != NULL; i++){
      if (!strcmp(tr[i],"cr")) mode|=O_CREAT;
      else if (!strcmp(tr[i],"ex")) mode|=O_EXCL;
      else if (!strcmp(tr[i],"ro")) mode|=O_RDONLY; 
      else if (!strcmp(tr[i],"wo")) mode|=O_WRONLY;
      else if (!strcmp(tr[i],"rw")) mode|=O_RDWR;
      else if (!strcmp(tr[i],"ap")) mode|=O_APPEND;
      else if (!strcmp(tr[i],"tr")) mode|=O_TRUNC; 
      else break;
    }

    return mode;
}



//***********************************/**********
/*************OPERACIONES************************/


// Inicializamos la tabla de ficheros con los valores estándar (0, 1, 2):
void InicializarTablaFicheros() {
    
    int i, modo_real;

    // Insertamos los descriptores del 0 al 19 (16 libres):
    for (i = 0; i < 20; i++) {
        tFicheroAbierto *f = malloc(sizeof(tFicheroAbierto));    // Asignamos un espacio de memoria al fichero.
        if (f == NULL) return;

        f->df = i;                                               // Asignamos su descriptor de archivo.
        modo_real = fcntl(i, F_GETFL);                           // Preguntamos al SO si existe

        
        if (modo_real != -1) {                                   

            if (i == 0) strcpy(f->nombre, "entrada estandar");
            else if (i == 1) strcpy(f->nombre, "salida estandar");
            else if (i == 2) strcpy(f->nombre, "error estandar");
            else strcpy(f->nombre, "desconocido");
            
            f->mode = modo_real;

        } else {

            strcpy(f->nombre, "no usado");
            f->mode = -1;                                           // -1 indica que esta libre
        }

        AniadirElemento(TablaFicheros, f);                       // Lo añadimos a la tabla de ficheros.
    }
}

// Para añadir cualquier fichero a la tabla:
int AniadirFicheroAbierto(int df, char *nombre, int mode){

    // Declaramos el numero de descriptor del fichero
    tFicheroAbierto target;
    target.df = df;

    // Comprobamos si el descriptor es uno de los 20 de la lista:
    int pos = BuscarElemento(TablaFicheros, &target, CompararDescriptor);

    // Sobreescribimos descriptor libre:
    if (pos != -1) {     
        tFicheroAbierto *f = (tFicheroAbierto *)GetElementoAtPos(TablaFicheros, pos);
        strcpy(f->nombre, nombre);
        f->mode = mode;
        return 0;
    }

    // Si el descriptor es mayor que 19, creamos un nuevo elemento:
    tFicheroAbierto *f = malloc(sizeof(tFicheroAbierto));
    if (f == NULL) return -1;

    f->df = df;
    strcpy(f->nombre, nombre);
    f->mode = mode;

    //Posible error:
    if (AniadirElemento(TablaFicheros, f) == -1) {
        free(f);
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
    if (p == -1) return -1;    // Si no lo encuentra

    //Accedemos al elemento de la lista:
    tFicheroAbierto *item = (tFicheroAbierto *)GetElementoAtPos(TablaFicheros, p);

    // Si es uno de los primeros 20 descriptores (0-19), simplemente se marca el descriptor como libre:
    if (df < 20) {
        strcpy(item->nombre, "no usado");
        item->mode = -1;
    
    // Si el descriptor es mayor o igual a 20, si que se elimina directamente de la lista:
    } else {
        BorrarElementoAtPos(TablaFicheros, p);
    }

    return 0;
}

char * NombreFicheroDescriptor(int df){
    //Fichero auxiliar cuyo df es el buscado:
    tFicheroAbierto target;
    target.df = df;

    int p = BuscarElemento(TablaFicheros, &target, CompararDescriptor);
    if (p == -1) return NULL;  // Si no lo encuentra

    // Si lo encuentra:
    tFicheroAbierto *item = (tFicheroAbierto *)GetElementoAtPos(TablaFicheros, p);
    return item->nombre;
}

int ModoFicheroDescriptor(int df){
    tFicheroAbierto target;
    target.df = df;
    
    int p = BuscarElemento(TablaFicheros, &target, CompararDescriptor);
    if (p == -1) return -1;  
    
    tFicheroAbierto *item = (tFicheroAbierto *)GetElementoAtPos(TablaFicheros, p);
    return item->mode;
}


void ListarFicherosAbiertos(){

    tFicheroAbierto *item = (tFicheroAbierto *)GetPrimerElemento(TablaFicheros);
    char textoModo[128];

    while (item != NULL) {
        ModoATexto(item->mode, textoModo);
        if(strstr(textoModo, "UNKNOWN") != NULL) strcpy(textoModo, "");
        printf("Descriptor: %d, offset: (  ) -> %s %s\n", item->df, item->nombre, textoModo);
        item = (tFicheroAbierto *)GetSiguienteElemento(TablaFicheros);
    }
}

