#include "mem.h"


static LISTASIMPLE TablaMemoria;    // Aprovechamos listasimple para implementar la lista.

/*
// Conversion tipo a texto:
void TipoATexto(int type, char *out) {
    switch(type){
        case 2:
            strcpy(out, "mmap");
            break;
        case 1:
            strcpy(out, "shared");
            break;
        case 0:
            strcpy(out, "malloc");
            break;
        default:
            strcpy(out, "unknown");
            break;
    }
}

// Conversion texto a tipo:
int TextoATipo(char *cad, int type){

    if (cad == NULL) return -1;

    if (!strcmp(cad,"malloc")) type = 0;
    else if (!strcmp(cad,"shared")) type = 1;
    else if (!strcmp(cad,"mmap")) type = 2;

    return type;
}
*/

/* FUNCIONES COMPARADORAS: */

static int CompDireccion(void *p1, void *p2) {
    tBloqueMem* a = (tBloqueMem *) p1;
    unsigned long addr = (unsigned long) p2;

    return (a->addr != addr);
}

static int CompSharedKey(void *p1, void *p2) {
    tBloqueMem* a = (tBloqueMem *) p1;
    key_t key = *(key_t *) p2;

    if ((a->type == 1) && (a->info.key == key))
        return 0;

    return 1;
}

static int CompMmapFile(void *p1, void *p2) {
    tBloqueMem* a = (tBloqueMem *) p1;
    const char *name = (const char *) p2;

    if (a->type == 2)
        return strcmp(a->info.mmap.name, name);

    return 1;

}

static int CompMallocSize(void *p1, void *p2) {
    tBloqueMem* a = (tBloqueMem *) p1;
    size_t size = (size_t) p2;

    if ((a->type == 0) && (a->size == size)) 
        return 0;
    
    return 1;
}


/***************************************/
/*************OPERACIONES************************/


// Inicializa la lista:
void InicializarTablaMemoria(){
    TablaMemoria[0] = NULL;
}


// Funcion auxiliar privada para crear y reservar un bloque base:
static tBloqueMem* CrearBloqueBase(void *addr, size_t size, int type) {

    tBloqueMem *b = (tBloqueMem *) malloc(sizeof(tBloqueMem));
    if (b == NULL) 
        return NULL;

    b->addr = (unsigned long) addr;
    b->size = size;
    b->time = time(NULL);           // Coge la hora del sistema a la hora de haber sido creado
    b->type = type;
    // Campo info se definira en la propia funcion de insercion.

    return b;
}


// Inserta en Malloc:
int InsertarBloqueMalloc(void *addr, size_t size){
    // Asignacion de memoria:
    tBloqueMem *b = CrearBloqueBase(addr, size, 0);
    if (b == NULL) return -1;

    // Dejamos campo info vacio

    return AniadirElemento(TablaMemoria, b);  // Llamamos a la funcion insertiva de listasimple

}


// Inserta memoria compartida:
int InsertarBloqueShared(void *addr, size_t size, key_t key){
    // Asignacion de memoria:
    tBloqueMem *b = CrearBloqueBase(addr, size, 1);
    if (b == NULL) return -1;

    b->info.key = key;   // La info adicional es la clave

    return AniadirElemento(TablaMemoria, b);
}


// Inserta mapeo de fichero:
int InsertarBloqueMmap(void *addr, size_t size, int fd, const char *name){
    // Asignacion de memoria:
    tBloqueMem *b = CrearBloqueBase(addr, size, 2);
    if (b == NULL) return -1;

    // Copia de datos:
    b->info.mmap.fd = fd;
    if (name != NULL) {
        strncpy(b->info.mmap.name, name, sizeof(b->info.mmap.name) - 1);
        b->info.mmap.name[sizeof(b->info.mmap.name) - 1] = '\0';

    } else
        b->info.mmap.name[0] = '\0';

    return AniadirElemento(TablaMemoria, b);
}


// Busqueda por Direccion:
// IMPORTANTE: Se le pasa un puntero a void para abstraer (y generalizar) el tipo de la direccion.
tBloqueMem* BuscarBloquePorDireccion(void *addr) {
    
    // Nos valemos de BuscarElemento de listasimple 
    // y el comparador de direcciones previamente implementado:
    // Hacemos un doble casteo a la direccion pasandolo como numerico y volviendolo a pasar a puntero
    int pos = BuscarElemento(TablaMemoria, (void *)(unsigned long)addr, CompDireccion); 
    if (pos == -1) 
        return NULL;

    // Si no hay errores, accedemos al elemento con GetElementoAtPos de listasimple:
    return (tBloqueMem*) GetElementoAtPos(TablaMemoria, pos);
}


// Busqueda por clave de memoria compartida:
tBloqueMem* BuscarBloqueSharedPorClave(key_t key){

    int pos = BuscarElemento(TablaMemoria, &key, CompSharedKey);
    if (pos == -1) 
        return NULL;

    return (tBloqueMem*) GetElementoAtPos(TablaMemoria, pos);
}


// Busqueda por nombre de fichero mapeado:
tBloqueMem* BuscarBloqueMmapPorFichero(const char *name){
    if (name == NULL) 
        return NULL;

    int pos = BuscarElemento(TablaMemoria, (void *)name, CompMmapFile);
    if (pos == -1) 
        return NULL;

    return (tBloqueMem*) GetElementoAtPos(TablaMemoria, pos);
}


// Busqueda por tamanho:
tBloqueMem* BuscarBloqueMallocPorTamanho(size_t size){
    int pos = BuscarElemento(TablaMemoria, &size, CompMallocSize);
    if (pos == -1) return NULL;
    return (tBloqueMem*) GetElementoAtPos(TablaMemoria, pos);
}


// Busqueda de un fichero en los mapeados (para close -f):
int EsDescriptorMapeado(int fd){
    int i;
    tBloqueMem *b;

    // Recorremos la lista en busca del descriptor:
    for(i = 0; (b = (tBloqueMem *)GetElementoAtPos(TablaMemoria, i)) != NULL; i++){
        if((b->type == 2) && (b->info.mmap.fd == fd))    // Si es mmap y coincide el descriptor:
            return 1;
    }
    return 0;

}


// Borra en base a la direccion del bloque:
int BorrarBloquePorDireccion(void *addr){
    return BorrarElemento(TablaMemoria, (void *)(unsigned long)addr, CompDireccion);
}


// Imprime un bloque individual:
void ImprimirBloqueMemoria(void *p){
    tBloqueMem *b = (tBloqueMem *) p;
    if(b == NULL) return;

    // Calculo de la hora del allocate:
    struct tm *tm = localtime(&b->time);
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%b %d %H:%M", tm);

    // Impresion del bloque:
    printf("\t0x%ld %12lu %s ", b->addr, (unsigned long)b->size, buffer);
    // %lx imprime en hexadecimal y %lu en decimal
    // Casteamos size para evitar posibles warnings

    // Impresion adicional:
    if(b->type == 1)       // Si es SHARED:
        printf("shared (key %d)\n", b->info.key);
    else if(b->type == 2)  // Si es MMAP:
        printf("%s (descriptor %d)\n", b->info.mmap.name, b->info.mmap.fd);
    else if(b->type == 0)  // Si es MALLOC:
        printf("malloc \n");
    else                   // Tipo desconocido:
        printf("unknown \n");
}


// Imprime la tabla filtrando por tipo o mostrando todos:
void ImprimirTablaMemoriaFiltrada(int type, int all){

    printf("******Lista de bloques asignados para el proceso %d\n", getpid());

    // Si se imprime la lista completa:
    if (all) {
        // Llamamos a la funcion de listasimple:
        ImprimirListaCompleta(TablaMemoria, 0, ImprimirBloqueMemoria);
    
    } else {
        // Si hay filtro por tipo (malloc, shared o mmap), hay que iterar:
        int i;
        tBloqueMem *b;

        for (i = 0; (b = (tBloqueMem *)GetElementoAtPos(TablaMemoria, i)) != NULL; i++) {

            if (b->type == type)   // Condicion de tipo:
                ImprimirBloqueMemoria(b);
        }
    }

}


// Borra la lista:
void VaciarTablaMemoria(){
    BorrarLista(TablaMemoria);
}