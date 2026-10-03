#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


#include "ejemplo.h"

// Tamaños máximos
#define MAXENTRADA 2048
#define MAXPROMPT 128

//Prompt por defecto:
char prompt[MAXPROMPT] = "->";

struct COMANDO{
   char * nombre;
   void (*funcion)(char**);
   char *arguments;          // Añadimos strings de argumentos y ayudas para comando help.
   char *help; 
};

/**************************SHELL**************************/

static struct COMANDO C[]={
   {"prompt", Cmd_prompt, "[cad]", "Cambia el prompt (simbolo de aviso) del shell a la cadena cad:\n\t%%u: nombre usuario\t%%p: pid del shell\n\t%%d: directorio actual\t%%D: directorio actual (contraido)\n\t%%B: Ultimo componente del directorio actual\n\t%%m: nombre maquina\t%%o: nombre del S.O.\n\t%%t el tabulador\t%%n: fin de linea\n\t%%%%: el caracter %%\t%%b: espacio en blanco"}
   {"exit",Cmd_fin, "", "Termina la ejecucion del shell"},
   {"bye",Cmd_fin, "", "Termina la ejecucion del shell"},
   {"date",Cmd_date, "[-d|-t]"	"Muestra la fecha y/o la hora actual"},
   {"pid",Cmd_pid, "[-p]",	"Muestra el pid del shell o de su proceso padre"},
   {"authors",Cmd_autores, "[-n|-l]",	"Muestra los nombres y/o logins de los autores"},
   {"sysinfo",Cmd_sysinfo, "", "Muestra informacion de la maquina donde corre el shell"},
   {"help",Cmd_help, "[cmd|-lt|-T|-all]",	"Muestra ayuda sobre los comandos\n\t-lt: lista topics de ayuda\n\t-T topic: lista comandos sobre ese topic\n\tcmd: info sobre el comando cmd\n\t-all: lista todos los topics con sus comandos"},
   {"chdir",Cmd_chdir, "[dir]", "Cambia (o muestra) el directorio actual del shell"},
   /*{"open",Cmd_open},
   {"close",Cmd_close},
   {"listopen",Cmd_listopen},
   {"dup",Cmd_dup},
   {"lseek",Cmd_lseek},
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

void DecidirComando (char *tr[])
{
  int i;

  if (tr[0]==NULL) return; /*superfluo, por si cambio otras cosas */
  for (i=0; C[i].nombre!=NULL; i++) 
    if (!strcmp(C[i].nombre,tr[0])){
        (*C[i].funcion)(tr+1);
	    return;
    }
  Cmd_pplano(tr); /*si no es un comando, es un  ejecutable externo: en primer plano*/
}

int TrocearCadena(char * cadena, char * trozos[])
{
  int i=1;
  if ((trozos[0]=strtok(cadena," \n\t"))==NULL)
      return 0;
  while ((trozos[i]=strtok(NULL," \n\t"))!=NULL)
     i++;
  return i;
}	
void ProcesarEntrada(char * entrada)
{
   char *tr[MAXENTRADA/2];
   if (TrocearCadena(entrada,tr)==0) /*no hay nada*/
	return;
   DecidirComando(tr);
}

int  main(int argc, char *argv[], char *ent[])
{
   char entrada[MAXENTRADA];

   if (argv[1]==NULL)
        printf ("Ejecutando con path vacio: %s -p para importar el path\n",argv[0]);
   else if (!strcmp(argv[1],"-p"))
        Cmd_importpath(NULL);

   while (1){
      printf ("%s", prompt);
      fgets(entrada,MAXENTRADA,stdin);
      ProcesarEntrada(entrada);
   }
}
