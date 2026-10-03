#include "ejemplo.h"
#include "Shellv4.c"



/***********************************/**********
/*************GESTIÓN DEL PATH / PROCESOS************************/

void AniadirAlPath(char *dir)
{
    if (dir==NULL)
        PathPrint();
    else if (PathAdd(dir)==-1)
        perror("Imposible aniadir");
}

void EliminarDelPath(char *dir)
{
    if (dir==NULL)
        PathPrint();
    else if (PathDel(dir)==-1)
        perror("Imposible eliminar");
}


void MostrarDirActual()
{
   char dir[MAXNOMBRE];
   
    if (getcwd(dir,MAXNOMBRE)==NULL)
	perror("Imposible obtener directorio");
    else
       printf ("%s\n",dir);
}

int ComprobarSegundoPlano (char *tr[])
{
    int i;
    for (i=0; tr[i]!=NULL;i++)
        if (!strcmp(tr[i],"&")){ /*& indica segundo plano*/ 
            tr[i]=NULL;         /*es el ultimo argumento*/
            return i;       /*si solo hay un & no se ejecuta nada en pplano*/
            }
    return 0;
}


void Proceso (char *tr[], int splano)
{
   pid_t pid;
   void Cmd_exec (char **);
   int background=splano || ComprobarSegundoPlano(tr);
   if ((pid=fork())==-1){
        perror ("Imposible crear proceso");
        return;
        }
  if (pid==0){  /*proceso hijo*/
    Cmd_exec (tr);
    exit(255); /*por si falla exec*/
    }
  if (!background) 
    waitpid(pid,NULL,0);
}


/*********************************************/
/*************COMANDOS DEL SHELL************************/

void Cmd_prompt(char * arg[]){

    // No se le pasa nuevo prompt.
    if (arg[0] == NULL) {
        printf("->  --->  ->");
        return;
    
    // Se le pasa un prompt:
    char cad[MAXPROMPT];
    }else if(arg[0][0] == '%'){                           // Prompts especiales:
        struct passwd pw = getpwuid(getuid());            // Usamos llamadas al sistema para obtener datos del usuario
                if(!pw){
                    perror("No se pudo obtener el nombre de usuario");
                    return;
                }
        switch(arg[0][1]){
            case 'u':
                strncpy(cad, pw.pw_name, MAXPROMPT - 2);
                break;

            case 'p':
                snprintf(cad, sizeof(cad), "%d", getpid());  // Pasa el PID a string
                break;

            case 'd':
                if (getcwd(cad, sizeof(cad)) == NULL){               //Asignamos cad a la ruta de dierectorio completa
                    perror("Error al obtener el directorio");
                    return;
                }
                break;

            case 'D':
                if (getcwd(cad, sizeof(cad)) == NULL){
                    perror("Error al obtener el directorio");
                    return;
                }
                char *cad2 = strstr(cad, pw.pw_name);                // Redefinimos cad desde la primera vez que aparece nombre usuario (= directorio personal)
                strcpy(cad, strcat("~", cad2 + strlen(pw.pw_name))); // Sustituimos el directorio personal por '~'         
                break;

            case 'B':
                
                if (getcwd(cad, sizeof(cad)) == NULL){
                    perror("Error al obtener el directorio");
                    return;
                }
                char *cad2 = strrchr(cad, '/');
                strcpy(cad, cad2 + 1);                               // Redefinimos cad como la última vez que aparece '/' en el cad anterior (sin '/')
                break;

            case 'm':
                if (gethostname(cad, sizeof(cad))){
                    perror("Error al obtener el nombre de la maquina");
                    return;
                }
                break;
            case 'o':
                struct utsname buffer;
                if (uname(&buffer)){
                    perror("Error al obtener la información del sistema");
                    return;
                }
                strcpy(cad, buffer.sysname);
                break;
            
            case 't':
                strcpy(cad, "\t");
                break;

            case 'n':
                strcpy(cad, "\n");
                break;

            case '%':
                strcpy(cad, "%%");
                break;

            case 'b':
                strcpy(cad, " ");
                break;

            default:
                perror("Error al generar el prompt, se mantiene el antiguo");
        }

    }else strcpy(cad, arg[0]);         // Prompt custom

    strcpy(prompt, cad, MAXPROMPT - 2); 
    prompt[MAXPROMPT - 2] = '\0';
}

void Cmd_fin (char * arg[])  /*todos los cmd_ comparten prototipo*/
{                            /*reciben los mismos parametros aunque no los usen*/
    exit(0);
}

void Cmd_autores(char *arg[]) //si arg[0] == NULL, solo se escribe autores
{
    if(arg[0] == NULL){ //si solo se pasa autores
        printf ("diego: diego.lopez.lopez1@udc.es\nbruno: bruno.gfarina@udc.es\npablo: pablo.muradas@udc.es\n");
    }
    else if (!strcmp(arg[0], "-l")){
        printf ("diego.lopez.lopez1@udc.es\nbruno.gfarina@udc.es\npablo.muradas@udc.es\n");
    }
    else if (!strcmp(arg[0], "-n")){
        printf ("diego\nbruno\npablo\n");
    }
    
}

void Cmd_date (char *arg[]){

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char buffer[100];

    if(arg[0] == NULL){
        strftime(buffer, sizeof(buffer), "%H:%M:%S\n%d/%m/%Y", tm);
        printf("%s\n", buffer);
    }
    else if (!strcmp(arg[0], "-d")){
        strftime(buffer, sizeof(buffer), "%d/%m/%Y", tm);
        printf("%s\n", buffer);
    }
    else if (!strcmp(arg[0], "-t")){
        strftime(buffer, sizeof(buffer), "%H:%M:%S", tm);
        printf("%s\n", buffer);
    }
    
}

void Cmd_pid (char * arg[])
{
    if (arg[0]==NULL)
        printf ("Pid de shell: %d\n",(int) getpid());
    else if (!strcmp (arg[0],"-p"))
        printf ("Pid del padre del shell: %d\n",(int) getppid());
}

void Cmd_help (char *arg[]){

    int i;

    //Sólo se pasa "help":
    if (arg[0] == NULL){
        int it;
        char *flim[2] = {"exit", NULL};

        printf("Comandos disponibles:");

        for (it = 0; it <= 1; it++){
            printf("\nCmds practica %d: ", it);
            for (i = 0; strcmp(C[i].nombre, flim[it]) != 0; i++) printf("%s", C[i].nombre);
        }

        return;
    }
    
    //Vemos de qué comando se trata:
    for (i=0; C[i].nombre != NULL; i++){
        if (!strcmp(C[i].nombre, arg[0])){
            printf("%s: %s\t%s", C[i].nombre, C[i].arguments, C[i].help);
            return;
        }
    }
    
    // Si no lo ha encontrado:
    printf("%s: no encontrado", arg[0]);

}

void Cmd_sysinfo (char *arg[]){
    struct utsname info;                                                              // Declaramos un tipo de struct utsname

    if (uname(&info) == -1) {                                                         // No se puede acceder a la información del sistema
        perror("Error al obtener la información del sistema (sysinfo)");
        return;
    }

    // Impresión de campos de info:
    printf("%s %s %s %s %s\n", info.sysname, info.nodename, info.release, info.version, info.machine);

}

void Cmd_chdir (char * arg[])
{
   if (arg[0]==NULL)
      MostrarDirActual();
   else if (chdir(arg[0])==-1)
      perror("Imposible cambiar directorio");
}

void Cmd_open(char * arg[]){/*
    
    int i,df, mode=0;
    
    if (tr[0]==NULL) { /*no hay parametro*//*
       ..............ListarFicherosAbiertos...............
        return;
    }
    for (i=1; tr[i]!=NULL; i++)
      if (!strcmp(tr[i],"cr")) mode|=O_CREAT;
      else if (!strcmp(tr[i],"ex")) mode|=O_EXCL;
      else if (!strcmp(tr[i],"ro")) mode|=O_RDONLY; 
      else if (!strcmp(tr[i],"wo")) mode|=O_WRONLY;
      else if (!strcmp(tr[i],"rw")) mode|=O_RDWR;
      else if (!strcmp(tr[i],"ap")) mode|=O_APPEND;
      else if (!strcmp(tr[i],"tr")) mode|=O_TRUNC; 
      else break;
      
    if ((df=open(tr[0],mode,0777))==-1)
        perror ("Imposible abrir fichero");
    else{
        ...........AnadirAFicherosAbiertos (descriptor...modo...nombre....)....
        printf ("Anadida entrada a la tabla ficheros abiertos..................",......);
    
*/
}

void Cmd_close (char * arg[]){/*
    int df;
    
    if (tr[0]==NULL || (df=atoi(tr[0]))<0) { /*no hay parametro*//*
      ..............ListarFicherosAbiertos............... *//*o el descriptor es menor que 0*//*
        return;
    }

    
    if (close(df)==-1)
        perror("Inposible cerrar descriptor");
    else
       ........EliminarDeFicherosAbiertos......
*/
}

void Cmd_listopen (char * arg[]);

void Cmd_dup (char *arg[]){/*
    int df, duplicado;
    char aux[MAXNAME],*p;
    
    if (tr[0]==NULL || (df=atoi(tr[0]))<0) { /*no hay parametro*//*
        ......ListarFicherosAbiertos........         *//*o el descriptor es menor que 0*//*
        return;
    }
    
 
    p=.....NombreFicheroDescriptor(df).......;
    sprintf (aux,"dup %d (%s)",df, p);
    .......AnadirAFicherosAbiertos......duplicado......aux.....fcntl(duplicado,F_GETFL).....;
*/
}

void Cmd_lseek (char *args[]);
void Cmd_readstr (char *args[]);
void Cmd_writestr (char *args[]);
void Cmd_makefile (char *args[]);
void Cmd_makedir (char *args[]);
void Cmd_delete (char *args[]);
void Cmd_deltree (char *args[]);
void Cmd_listfile (char *args[]);
void Cmd_list (char *args[]);

//comandos que no están en la practica pero son imprescindibles para el funcionamiento:
void Cmd_exec (char *arg[])
{
  if (execv(Ejecutable(arg[0]),arg)==-1)
	perror ("Imposible ejecutar");
}

/* void Cmd_splano (char *arg[])
{
  Proceso (arg,1);
}   no hace falta??*/

void Cmd_pplano (char *arg[])
{
  Proceso(arg,0);
}

/* void Cmd_path(char *arg[])
{
    if (arg[0]==NULL)
        PathPrint();
    else if (!strcmp(arg[0],"-add"))
        AniadirAlPath (arg[1]);
    else if (!strcmp(arg[0],"-del"))
        EliminarDelPath (arg[1]);
    else if (!strcmp(arg[0],"-show"))
        PathPrint ();
    else if (!strcmp(arg[0],"-clear"))
        PathClear ();
    else if (!strcmp(arg[0],"-import"))
        PathAddPath ();
    else printf ("Opciones validas: -add|-del|-show|-clear|-import\n");
} no hace falta?? */

void Cmd_importpath (char *arg[])
{
    PathAddPath();
}




