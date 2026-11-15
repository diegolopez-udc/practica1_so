#ifndef PROCESOS_H
#define PROCESOS_H

#include <unistd.h>
#include <pwd.h>
#include <signal.h>        // Para senales de control de procesos
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include <errno.h>

#define MAXDATE 64
#define MAXCOMMAND 2048

#include "listasimple.h"


// Definimos tipo de struct para senales:
struct SEN{
    char *nombre;
    int senal;
};

// Operaciones de senhales:
int ValorSenal(char *sen);
char *NombreSenal(int sen);


// Definimos el nodo de la lista de procesos:

typedef enum {                  // Un enum para el estado del proceso:
    ACTIVE,
    FINISHED,
    SIGNALED,
    STOPPED
}tEstadoProceso;

typedef struct {
    pid_t pid;
    uid_t uid;                  // UID efectivo con el que se lanzó el proceso
    time_t date;
    tEstadoProceso state;
    int svalue;
    char comando[MAXCOMMAND];
}tProceso;

/* *** OPERACIONES *** */
int AniadirProcesoBackground(pid_t pid, char *arg[]);
void ListarProcesosBackground(void);
void BorrarProcesosBackground(int term, int sig);
void VaciarListaProcesos(void);


#endif