#include "ejemplo.h"



/****************************************/
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
        printf("->  --->  ->\n");
        return;

    }
    // Se le pasa un prompt:
    char cad[MAXPROMPT];
    if(arg[0][0] == '%'){                                  // Prompts especiales:
        struct passwd *pw = getpwuid(getuid());            // Usamos llamadas al sistema para obtener datos del usuario
                if(!pw){
                    perror("No se pudo obtener el nombre de usuario");
                    return;
                }
        switch(arg[0][1]){
            case 'u':
                strncpy(cad, pw->pw_name, MAXPROMPT - 2);
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
                char dir_actual[MAXPROMPT];
                if (getcwd(dir_actual, sizeof(dir_actual)) == NULL) {
                    perror("Error al obtener el directorio");
                    return;
                }
                
                size_t len_home = strlen(pw->pw_dir);
                // Si el directorio actual empieza por la ruta de pw_dir (/home/usuario)
                if (strncmp(dir_actual, pw->pw_dir, len_home) == 0) {
                    snprintf(cad, sizeof(cad), "~%s", dir_actual + len_home);
                } else {
                    strncpy(cad, dir_actual, sizeof(cad) - 1);
                }
                break;


            case 'B':
                
                if (getcwd(cad, sizeof(cad)) == NULL){
                    perror("Error al obtener el directorio");
                    return;
                }
                char *cad3 = strrchr(cad, '/');
                strcpy(cad, cad3 + 1);                               // Redefinimos cad como la última vez que aparece '/' en el cad anterior (sin '/')
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
                strcpy(cad, "%");
                break;

            case 'b':
                strcpy(cad, " ");
                break;

            default:
                printf("Error al generar el prompt, se mantiene el antiguo\n");
                return;
        }

    }else strcpy(cad, arg[0]);         // Prompt "custom"

    strncpy(prompt, cad, MAXPROMPT - 2); 
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

    //Solo se pasa "help":
    if (arg[0] == NULL){
        printf("Comandos de la Practica 0: prompt\n");
        printf("Comandos de la Practica 1: exit bye date pid authors sysinfo help chdir open close listopen dup lseek readstr writestr makefile makedir delete deltree listfile list\n");
        
        return;
    }
    
    //Vemos de que comando se trata:
    for (i=0; C[i].nombre != NULL; i++){
        if (!strcmp(C[i].nombre, arg[0])){
            printf("%s: %s\t%s\n", C[i].nombre, C[i].arguments, C[i].help);
            return;
        }
    }
    
    // Si no lo ha encontrado:
    printf("%s: no encontrado\n", arg[0]);

}

void Cmd_sysinfo (char *arg[]){
    struct utsname info;                                                              // Declaramos un tipo de struct utsname

    if (uname(&info) == -1) {                                                         // No se puede acceder a la información del sistema
        perror("Error al obtener la información del sistema (sysinfo)");
        return;
    }

    // Impresion de campos de info:
    printf("%s %s %s %s %s\n", info.sysname, info.nodename, info.release, info.version, info.machine);

}

void Cmd_chdir (char * arg[])
{
   if (arg[0]==NULL)
      MostrarDirActual();
   else if (chdir(arg[0])==-1)
      perror("Imposible cambiar directorio");
}

void Cmd_open(char * arg[]){
    int df;
    int mode = 0;
    
    // Solo se pasa "open":
    if (arg[0]==NULL) {
        ListarFicherosAbiertos();
        return;
    }
    
    // Convertimos a modos los parametros pasados:
    mode = TextoAModo(arg + 1, mode);
    
    if ((df=open(arg[0],mode,0777))==-1)
        perror ("Imposible abrir fichero");
    else{
        if(AniadirFicheroAbierto(df, arg[0], mode) == -1) perror("Error al añadir el fichero a la lista de Ficheros Abiertos\n");
        else printf ("Anadida entrada %d a la tabla ficheros abiertos\n", df);
    }
}

void Cmd_close (char * arg[]){
    int df;
    
    // Si sólo se pasa "close" o el descriptor es menor que 0:
    if (arg[0]==NULL || (df=atoi(arg[0]))<0) { 
        ListarFicherosAbiertos(); 
        return;
    }


    // HACER PARAMETRO -f !!! ==> Practica 2

    if (close(df)==-1)
        perror("Imposible cerrar descriptor");
    else
        EliminarFicheroAbierto(df);


}

void Cmd_listopen (char * arg[]){
    ListarFicherosAbiertos();
}

void Cmd_dup (char *arg[]){
    int df, duplicado, mode;  
    char aux[MAXNOMBRE],*p;
    
    if (arg[0]==NULL || (df=atoi(arg[0]))<0) { //no hay parametro
        ListarFicherosAbiertos();         //o el descriptor es menor que 0
        return;
    }
    
    if((duplicado = dup(df)) == -1){
        perror("Imposible duplicar descriptor");
        return;
    }
    if((p = NombreFicheroDescriptor(df)) == NULL){  //este error en teoria, si está bien implementado el programa no se deberia dar; comprobar al acabar la practica
        printf("Error: el descriptor %d no existe en la tabla interna.\n", df);
        return;
    }

    sprintf(aux,"duplicado de %d (%s)",df, p);
    mode = ModoFicheroDescriptor(df);

    AniadirFicheroAbierto(duplicado, aux, mode);    

}

void Cmd_lseek (char *args[]){
    int df;

    // Falta de parámetros:
    if (args[0] == NULL || args[1] == NULL || args[2] == NULL || (df=atoi(args[0])) < 0) {
        printf("Parametros incorrectos\n");
        return;
    }

    // Inicializamos variables de offset y referencia:
    off_t pos = (off_t) atol(args[1]);
    int ref;

    // Codificamos la referencia en una mascara:
    if (!strcmp(args[2], "SEEK_SET")) {
        ref = SEEK_SET;
    } else if (!strcmp(args[2], "SEEK_CUR")) {
        ref = SEEK_CUR;
    } else if (!strcmp(args[2], "SEEK_END")) {
        ref = SEEK_END;
    }else{   // lseek() permite la referencia como número (0, 1, 2)
        ref = atoi(args[2]);
        // Casos anomalos los acotamos a SEEK_SET
        if (ref < 0 || ref > 2) ref = 0;
    }

    // Llamada al sistema:
    off_t new_offset = lseek(df, pos, ref);

    // Impresión de resultados:
    if (new_offset == -1){
        char mensaje_error[256];
        sprintf(mensaje_error, "Error al intentar posicionar el descriptor %d en el offset %ld", df, pos);
        perror(mensaje_error);
    }else 
        printf("Descriptor %d posicionado en %ld\n", df, pos);


}

void Cmd_readstr (char *args[]);
void Cmd_writestr (char *args[]);
void Cmd_makefile (char *args[]);
void Cmd_makedir (char *args[]);


void Cmd_delete (char *args[]){
    int i;

    // Para saber el directorio actual:
    char cadena[1024];
    if (getcwd(cadena, sizeof(cadena)) == NULL) // Si no puede acceder, sale de la funcion.
        return;
    char *diractual = strrchr(cadena, '/');

    // Se pasa solo "delete":
    if (args[0] == NULL) {
        printf("%s\n", diractual);
        return;
    }

    // Recorre los ficheros/directorios pasados:
    for (i = 0; args[i] != NULL; i++){
        struct stat st;

        //Examina el fichero/directorio:
        if (lstat(args[i], &st) == -1) {       // El elemento no existe o no hay permisos
            char mensaje_error[256];
            sprintf(mensaje_error, "Imposible borrar %s", args[i]);
            perror(mensaje_error);
            continue;

        } else {
            if (S_ISDIR(st.st_mode)) {         // Es un directorio
                if (rmdir(args[i]) == -1){     // Si no esta vacio / error de borrado:
                    char mensaje_error[256];
                    sprintf(mensaje_error, "Imposible borrar %s", args[i]);
                    perror(mensaje_error);
                    continue;
                }

            } else {                           // Es un fichero normal
                if (unlink(args[i]) == -1){    // Error de borrado: Aunque el fichero exista, puede dar error la llamada unlink().
                    char mensaje_error[256];
                    sprintf(mensaje_error, "Imposible borrar %s", args[i]);
                    perror(mensaje_error);
                    continue;
                }
            }
        }
    }
}

void Cmd_deltree (char *args[]);
void Cmd_listfile (char *args[]);
void Cmd_list (char *args[]);

//Comandos que no están en la practica pero son imprescindibles para el funcionamiento:
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




