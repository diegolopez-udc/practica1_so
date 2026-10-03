#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


#include "ejemplo.h"


#define MAXENTRADA 2048

struct COMANDO{
  char * nombre;
  void (*funcion)(char**);
};

/**************************SHELL**************************/

static struct COMANDO C[]={
   {"exit",Cmd_fin},
   {"bye",Cmd_fin},
   {"date",Cmd_date},
   {"pid",Cmd_pid},
   {"authors",Cmd_autores},
   /*{"sysinfo",Cmd_sysinfo},
   {"help",Cmd_help},
   {"chdir",Cmd_chdir},
   {"open",Cmd_open},
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
   {NULL,NULL},
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
      printf ("-> ");
      fgets(entrada,MAXENTRADA,stdin);
      ProcesarEntrada(entrada);
   }
}
