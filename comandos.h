#ifndef COMANDOS_H
#define COMANDOS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>    

#include <unistd.h>           // Permite hacer llamadas al sistema
#include <time.h>             // Formatear fechas del sistema
#include <dirent.h>           // Abrir, leer y cerrar directorios en el sistema de archivos
#include <pwd.h>              // Proporciona acceso a la base de datos de cuentas de usuario y contrasenhas de sistemas UNIX
#include <grp.h>              // Acceder a la base de datos de grupos de usuarios del SO
#include <sys/wait.h>         // Permite que un proceso padre espere a que terminen/cambien de estado sus procesos hijo
#include <sys/types.h>        // Definiciones de tipos de datos primitivos del sistema 
#include <sys/utsname.h>      // Permite obtener informacion sobre el SO y el HW en el que se ejecuta

#include "path.h"
#include "mem.h"
#include "procesos.h"

#define MAXNOMBRE 1024        // Tamanho maximo nombre de comando
#define MAXSTACK 2048         // Tamanho maximo de la pila

/*************VARIABLES DEL SHELLv4:***************/

// Tamanhos maximos
#define MAXENTRADA 2048
#define MAXPATH 1024
#define MAXMERROR 256
#define MAXVAR 2048           // Asignamos este tamanho para que quepa en una entrada y no desborde el buffer de la pila


// Tipo de struct COMANDO:
struct COMANDO{
   char *nombre;
   void (*funcion)(char**);
   char *arguments;           // Aniadimos strings de argumentos y ayudas para comando help.
   char *help; 
};


// Inicializacion environ para acceder a las variables de entorno:
extern char **environ;


/*********************************************/


// COMANDOS:
void Cmd_fin (char * arg[]);
void Cmd_autores(char *arg[]);
void Cmd_date (char *arg[]);
void Cmd_pid (char *arg[]);
void Cmd_help (char *arg[]);
void Cmd_sysinfo (char *arg[]);
void Cmd_chdir (char * arg[]);
void Cmd_open(char * arg[]);
void Cmd_close (char * arg[]);
void Cmd_listopen (char * arg[]);
void Cmd_dup (char *arg[]);
void Cmd_lseek (char *args[]);
void Cmd_readstr (char *args[]);
void Cmd_writestr (char *args[]);
void Cmd_makefile (char *args[]);
void Cmd_makedir (char *args[]);
void Cmd_delete (char *args[]);
void Cmd_deltree (char *args[]);
void Cmd_listfile (char *args[]);
void Cmd_list (char *args[]);

void Cmd_allocate(char *args[]);
void Cmd_deallocate(char *args[]);
void Cmd_io(char *args[]);
void Cmd_memory(char *args[]);
void Cmd_read(char *args[]);
void Cmd_write(char *args[]);
void Cmd_recurse(char *args[]);
void Cmd_memfill(char *args[]);
void Cmd_memdump(char *args[]);

void Cmd_getuid (char *arg[]);
void Cmd_setuid (char *arg[]);
void Cmd_prio (char *arg[]);
void Cmd_path(char *arg[]);
void Cmd_showenv (char *arg[]);
void Cmd_showvar (char *arg[]);
void Cmd_changevar (char *arg[]);
void Cmd_changevarWrong (char *arg[]);
void Cmd_fork (char *arg[]);
void Cmd_ejec (char *arg[]);
void Cmd_ejecpri (char *arg[]);
void Cmd_pplano (char *arg[]);
void Cmd_pplanopri(char *arg[]);
void Cmd_splano (char *arg[]);
void Cmd_splanopri (char *arg[]);
void Cmd_jobs (char *arg[]);
void Cmd_deljobs (char *arg[]);

void Cmd_exec (char *arg[]);
void Cmd_importpath (char *arg[]);


/*********************************************/


// Tipo de struct COMANDO:

static struct COMANDO C[]={
   {"exit",Cmd_fin, "", "Termina la ejecucion del shell"},
   {"bye",Cmd_fin, "", "Termina la ejecucion del shell"},
   {"date",Cmd_date, "[-d|-t]", "Muestra la fecha y/o la hora actual"},
   {"pid",Cmd_pid, "[-p]",	"Muestra el pid del shell o de su proceso padre"},
   {"authors",Cmd_autores, "[-n|-l]",	"Muestra los nombres y/o logins de los autores"},
   {"sysinfo",Cmd_sysinfo, "", "Muestra informacion de la maquina donde corre el shell"},
   {"help",Cmd_help, "[cmd]",	"Muestra ayuda sobre los comandos"},
   {"chdir",Cmd_chdir, "[dir]", "Cambia (o muestra) el directorio actual del shell"},
   {"open",Cmd_open, "fich m1 m2...", "Abre el fichero fich\n\ty lo anade a la lista de ficheros abiertos del shell\n\tm1, m2..es el modo de apertura (or bit a bit de los siguientes)\n\tcr: O_CREAT\tap: O_APPEND\n\tex: O_EXCL\tro: O_RDONLY\n\trw: O_RDWR\two: O_WRONLY\n\ttr: O_TRUNC"},
   {"close",Cmd_close, "[-f]", "Cierra el descriptor df y lo elimina de la lista de ficheros abiertos\n\t-f: cierra el descriptor aunque corresponada a un mapeo activo"},
   {"listopen",Cmd_listopen, "[n]", "Lista los ficheros abiertos (al menos n) del shell"},
   {"dup",Cmd_dup, "df","Duplica el descriptor de fichero df\n\ty anade una nueva entrada a la lista ficheros abiertos"},
   {"lseek",Cmd_lseek, "df off ref", "Posiciona el offset en df en off. ref es la referencia:\n\tSEEK_SET principio fichero\n\tSEEK_CUR posicion actual\n\tSEEK_END final del fichero"},
   {"readstr",Cmd_readstr, "df cont", "Lee cont bytes del fichero con descriptor df\n\ty los muestra en pantalla"},
   {"writestr",Cmd_writestr, "df string", "Escribe el string string en el fichero descrito por df"},
   {"makefile",Cmd_makefile, "[name]",	"Crea un fichero de nombre name"},
   {"makedir",Cmd_makedir, "[name]", "Crea un directorio de nombre name"},
   {"delete",Cmd_delete, "[name1 name2 ..]",	"Borra ficheros o directorios vacios"},
   {"deltree",Cmd_deltree, "[name1 name2 ..]", "Borra ficheros o directorios no vacios recursivamente"},
   {"listfile",Cmd_listfile, "[-long][-link][-acc] name1 name2...", "Lista ficheros.\n\t-long: listado largo\n\t-acc: acesstime\n\t-link: si es enlace simbolico, el path contenido"},
   {"list",Cmd_list, "[-reca][-recb][-hid][-long][-link][-acc] n1 n2...", "Lista contenidos de directorios\n\t-long: listado largo\n\t-hid: incluye los ficheros ocultos\n\t-recb: recursivo (antes)\n\t-reca: recursivo (despues)\n\t-acc: acesstime\n\t-link: si es enlace simbolico, el path contenido"},
   {"allocate", Cmd_allocate, "[-malloc|-shared|-createshared|-mmap]...", "Asigna un bloque de memoria\n\t-malloc tam: asigna un bloque malloc de tamano tam\n\t-createshared cl tam: asigna (creando) el bloque de memoria compartida de clave cl y tamano tam\n\t-shared cl: asigna el bloque de memoria compartida (ya existente) de clave cl\n\t-mmap fich perm: mapea el fichero fich, perm son los permisos"},
   {"deallocate", Cmd_deallocate, "[-malloc|-shared|-delkey|-mmap|addr]...", "Desasigna un bloque de memoria\n\t-malloc tam: desasigna el bloque malloc de tamano tam\n\t-shared cl: desasigna (desmapea) el bloque de memoria compartida de clave cl\n\t-delkey cl: elimina del sistema (sin desmapear) la clave de memoria cl\n\t-mmap fich: desmapea el fichero mapeado fich\n\taddr: desasigna el bloque de memoria en la direccion addr"},
   {"i-o", Cmd_io, "[read|write] [-o] fiche addr cont", "\n\tread fich addr cont: Lee cont bytes desde fich a addr\n\twrite [-o] fich addr cont: Escribe cont bytes desde addr a fich. -o para sobreescribir\n\t\taddr es una direccion de memoria"},
   {"memory", Cmd_memory, "[-blocks|-funcs|-vars|-all|-pmap]...", "Muestra muestra detalles de la memoria del proceso\n\t-blocks: los bloques de memoria asignados\n\t-funcs: las direcciones de las funciones\n\t-vars: las direcciones de las variables\n\t:-all: todo\n\t-pmap: muestra la salida del comando pmap(o similar)\n\t"},
   {"read", Cmd_read, "df addr cont", "Transfiere cont bytes del fichero\n\tdescrito por df a la direccion addr"},
   {"write", Cmd_write, "df addr cont", "Transfiere cont bytes desde la direccion addr\n\tal fichero descrito por df"},
   {"recurse", Cmd_recurse, "[n]", "Invoca a la funcion recursiva n veces"},
   {"memfill", Cmd_memfill, "addr cont byte", "Llena la memoria a partir de addr con byte"},
   {"memdump", Cmd_memdump, "addr cont", "Vuelca en pantallas los contenidos (cont bytes) de la posicion de memoria addr"},
   {"getuid", Cmd_getuid, "", "Muestra las credenciales del proceso que ejecuta el shell"},
   {"setuid", Cmd_setuid, "[-l] id", "Cambia las credenciales del proceso que ejecuta el shell\n\tid: establece la credencial al valor numerico id\n\t-l id: establece la credencial a login del usuario id"},
   {"prio", Cmd_prio, "[pid] [valor]", "Muestra o cambia la prioridad de un proceso pid a valor"},
   {"path", Cmd_path, "[-add|-del|-clear|-path]...", "Manipula o muestra la ruta de busqueda del shell (path)\n\t-add dir: aniade dir a la ruta de busqueda(equiv +dir)\n\t-del dir: elimina dir de la ruta de busqueda (equiv -dir)\n\t-clear: vacia la ruta de busqueda\n\t-path: importa el PATH en la ruta de busqueda"},
   {"showenv", Cmd_showenv, "[-environ|-addr]", "Muestra el entorno del proceso\n\t-environ: accede usando environ (en lugar del tercer arg de main)\n\t-addr: muestra el valor y donde se almacenan environ y el 3er arg main"},
   {"showvar", Cmd_showvar, "var", "Muestra el valor y las direcciones de la variable de entorno var"},
   {"changevar", Cmd_changevar, "[-a|-e|-p] var valor", "Cambia el valor de una variable de entorno\n\t-a: accede por el tercer arg de main\n\t-e: accede mediante environ\n\t-p: accede mediante putenv"},
   {"changevarWrong", Cmd_changevarWrong, "-a|-e|-p  -mo|-ma|-ms var valor", "Cambia de manera incorrecta el valor de una variable de entorno\n\t-mo sobreescribe la variable original\n\t-ma usa una variable automatica para crear la nueva variable\n\t-ms usa una variable estatica para crear la nueva variable\n"},
   {"fork", Cmd_fork, "", "El shell hace fork y queda en espera a que su hijo termine"},
   {"ejec", Cmd_ejec, "VAR1 VAR2 ..prog args....[@pri]", "Ejecuta, sin crear proceso,prog con argumentos\n\ten un entorno que contiene solo las variables VAR1, VAR2..."},
   {"ejecpri", Cmd_ejecpri, "prio prog args....", "Ejecuta, sin crear proceso, prog con argumentos con la prioridad cambiada a prio"},
   {"pplano", Cmd_pplano, "prog args...", "Crea un proceso que ejecuta en primer plano prog con argumentos args"},
   {"pplanopri", Cmd_pplanopri, "prio prog args...", "Crea un proceso que ejecuta en primer plano prog con argumentos args con la prioridad cambiada a prio"},
   {"splano", Cmd_splano, "prog args...", "Crea un proceso que ejecuta en segundo plano prog con argumentos args"},
   {"splanopri", Cmd_splanopri, "prio prog args...", "Crea un proceso que ejecuta en segundo plano prog con argumentos args con la prioridad cambiada a prio"},
   {"jobs", Cmd_jobs, "", "Lista los procesos en segundo plano"},
   {"deljobs", Cmd_deljobs, "[-term][-sig]", "Elimina los procesos de la lista procesos en sp\n\t-term: los terminados\n\t-sig: los terminados por senal"},
   {NULL,NULL, NULL, NULL},
};

#endif
