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

#define MAXNOMBRE 1024

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


#endif
