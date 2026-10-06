#include "ejemplo.h"

//VARIABLES GLOBALES (prompt por defecto):
char prompt[MAXPROMPT] = "-> ";

/**************************OPERACIONES**************************/

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


/******************PROGRAMA PRINCIPAL*********************/

int  main(int argc, char *argv[], char *ent[])
{
   //Definimos entrada
   char entrada[MAXENTRADA];

   //Inicializamos Tabla de Ficheros Abiertos para 0, 1, 2 (entrada/salida/error estandar)
   InicializarTablaFicheros();

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