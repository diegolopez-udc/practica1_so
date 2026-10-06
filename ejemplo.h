#ifndef EJEMPLO_H
#define EJEMPLO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#include <unistd.h>      // Permite hacer llamadas al sistema
#include <time.h>        // Formatear fechas del sistema
#include <dirent.h>      // Abrir, leer y cerrar directorios en el sistema de archivos
#include <pwd.h>         // Proporciona acceso a la base de datos de cuentas de usuario y contraseñas de sistemas UNIX
#include <grp.h>         // Acceder a la base de datos de grupos de usuarios del SO
#include <sys/wait.h>    // Permite que un proceso padre espere a que terminen/cambien de estado sus procesos hijo
#include <sys/types.h>   // Definiciones de tipos de datos primitivos del sistema 
#include <sys/utsname.h> // Permite obtener información sobre el SO y el HW en el que se ejecuta

#include "path.h"
#include "files.h"

//Tamaño máximo nombre de comando:
#define MAXNOMBRE 1024

/*************VARIABLES DEL SHELLv4:***************/

// Tamaños máximos
#define MAXENTRADA 2048
#define MAXPROMPT 128

//Prompt por defecto:
extern char prompt[MAXPROMPT];


// Tipo de struct COMANDO (I):

struct COMANDO{
   char * nombre;
   void (*funcion)(char**);
   char *arguments;          // Añadimos strings de argumentos y ayudas para comando help.
   char *help; 
};


/*********************************************/


// COMANDOS:
void Cmd_prompt(char * arg[]);
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

void Cmd_exec (char *arg[]);
void Cmd_pplano (char *arg[]);
void Cmd_importpath (char *arg[]);


/*********************************************/


// Tipo de struct COMANDO (II):

static struct COMANDO C[]={
   {"prompt", Cmd_prompt, "[cad]", "Cambia el prompt (simbolo de aviso) del shell a la cadena cad:\n\t%u: nombre usuario\t%p: pid del shell\n\t%d: directorio actual\t%D: directorio actual (contraido)\n\t%B: Ultimo componente del directorio actual\n\t%m: nombre maquina\t%o: nombre del S.O.\n\t%t el tabulador\t%n: fin de linea\n\t%%: el caracter %\t%b: espacio en blanco"},
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
   /*{"lseek",Cmd_lseek},
   {"readstr",Cmd_readstr},
   {"writestr",Cmd_writestr},
   {"makefile",Cmd_makefile},
   {"makedir",Cmd_makedir},
   {"delete",Cmd_delete},
   {"deltree",Cmd_deltree},
   {"listfile",Cmd_listfile},
   {"list",Cmd_list},*/
   {NULL,NULL, NULL, NULL},
  };

#endif
