#ifndef EJEMPLO_H
#define EJEMPLO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "path.h"

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
