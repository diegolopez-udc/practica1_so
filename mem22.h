#ifndef MEM_H
#define MEM_H

#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <sys/ipc.h>   // Para IPC (claves de memoria compartida)
#include <sys/shm.h>   // Para shmget, shmat, shmdt
#include <sys/mman.h>  // Para mmap, munmap

#include "files.h"

#define MAXNAME 1024

/*********************************/


// Definimos el struct para el info de ficheros mapeados:
typedef struct {
    int fd;
    char name[MAXNAME];
}tMmapInfo;

// Definimos el tipo del fichero:
typedef struct {
    unsigned long addr;   // Direccion del bloque
    size_t size;          // Tamanho del bloque
    time_t time;          // Fecha y hora de asignacion
    int type;             // Tipo de asignacion {0 = MALLOC, 1 = SHARED, 2 = MMAP}

    union {               // Informacion adicional: clave de memoria compartida (key_t) / nombre y desciptor
        key_t key;        // para ficheros mapeados (tMmapInfo)
        tMmapInfo mmap; 
    }info;               
                         
} tBloqueMem;

//OPERACIONES:
void TipoATexto(int type, char *out);
int TextoATipo(char *cad, int type);

void InicializarTablaMemoria();

int InsertarBloqueMalloc(void *addr, size_t size);
int InsertarBloqueShared(void *addr, size_t size, key_t key);
int InsertarBloqueMmap(void *addr, size_t size, int fd, const char *name);

tBloqueMem* BuscarBloquePorDireccion(void* addr);
tBloqueMem* BuscarBloqueSharedPorClave(key_t key);
tBloqueMem* BuscarBloqueMmapPorFichero(const char *name);
tBloqueMem* BuscarBloqueMallocPorTamanho(size_t size);

// Para close -f
int EsDescriptorMapeado(int fd);

int BorrarBloquePorDireccion(void *addr);

void ImprimirBloqueMemoria(void *p);
void ImprimirTablaMemoriaFiltrada(int type, int all);

void VaciarTablaMemoria();

#endif