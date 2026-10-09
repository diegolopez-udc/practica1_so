#include "comandos.h"



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
   char dir[MAXPATH];
   
    if (getcwd(dir, sizeof(dir)) == NULL)
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

// Para primera letra del codigo de permisos del fichero:
char LetraTF (mode_t m)
{
    switch (m&S_IFMT) { /*and bit a bit con los bits de formato,0170000 */
        case S_IFSOCK: return 's'; /*socket */
        case S_IFLNK: return 'l'; /*symbolic link*/
        case S_IFREG: return '-'; /* fichero normal*/
        case S_IFBLK: return 'b'; /*block device*/
        case S_IFDIR: return 'd'; /*directorio */ 
        case S_IFCHR: return 'c'; /*char device*/
        case S_IFIFO: return 'p'; /*pipe*/
        default: return '?'; /*desconocido, no deberia aparecer*/
    }
}


/*  Devuelven los permisos de un fichero en formato rwx---- 
    en base al campo st_mode del struct stat                 */

char * ConvierteModo (mode_t m, char *permisos)
{
    strcpy (permisos,"---------- ");
    
    permisos[0] = LetraTF(m);
    if (m&S_IRUSR) permisos[1]='r';    /*propietario*/
    if (m&S_IWUSR) permisos[2]='w';
    if (m&S_IXUSR) permisos[3]='x';
    if (m&S_IRGRP) permisos[4]='r';    /*grupo*/
    if (m&S_IWGRP) permisos[5]='w';
    if (m&S_IXGRP) permisos[6]='x';
    if (m&S_IROTH) permisos[7]='r';    /*resto*/
    if (m&S_IWOTH) permisos[8]='w';
    if (m&S_IXOTH) permisos[9]='x';
    if (m&S_ISUID) permisos[3]='s';    /*setuid, setgid y stickybit*/
    if (m&S_ISGID) permisos[6]='s';
    if (m&S_ISVTX) permisos[9]='t';
    
    return permisos;
}


int EsDirectorio (char * dir)          /*para saber si algo es directorio o no*/
{
    struct stat s;
    if (lstat(dir,&s) == -1){          /*si no puedo acceder: para mi no es directorio*/
        char mensaje_error[MAXMERROR];
        sprintf(mensaje_error, "Error al acceder a %s", dir);
        perror(mensaje_error);

        return 0;
    }

    return (S_ISDIR(s.st_mode));
}

// Función auxiliar para imprimir informacion de un fichero / enlace:
void imprimirElementoList(const char *nombre, const char *path, struct stat *st, 
                          int lon, int link, int acc) {
                            
    if (lon) {
        //Tiempo mostrado:
        time_t t;
        if (acc) t = st->st_atime;    // Depende acc, se toma tiempo de acceso o de modificacion.
        else t = st->st_mtime;
        struct tm *tm = localtime(&t);
        char buffer_time[100];
        strftime(buffer_time, sizeof(buffer_time), "%Y/%m/%d-%H:%M", tm);

        // Propietario y Grupo:
        struct passwd *pw = getpwuid(st->st_uid);
        struct group *gr = getgrgid(st->st_gid);
        char *user_name, *gr_name;
        if (pw == NULL || gr == NULL){
            user_name = "Desconocido";
            gr_name = "Desconocido";
        }else{
            user_name = pw->pw_name;
            gr_name = gr->gr_name;
        }

        // Para imprimir los permisos:
        char permisos[12];

        // Impresion:
        printf("%s %2ld (%8ld) %12s %12s %s %8ld %s", 
               buffer_time, st->st_nlink, st->st_ino, 
               user_name, gr_name, ConvierteModo(st->st_mode, permisos),
               st->st_size, nombre);
    }
    
    // Con -acc sin -long:           
    else if (acc) {

        time_t t = st->st_atime;
        struct tm *tm = localtime(&t);
        char buffer_time[100];
        strftime(buffer_time, sizeof(buffer_time), "%Y/%m/%d-%H:%M", tm);

        // Impresion:
        printf("\t%8ld %s %s", st->st_size, buffer_time, nombre);

    //Impresion normal:
    } else printf("\t%8ld %s", st->st_size, nombre);

    // Con los links. Comprombamos tambien que tiene un enlace simbólico con S_ISLNK:
    if (link && S_ISLNK(st->st_mode)) {
        char buffer_link[MAXNOMBRE];      // Buffer donde se guarda el nombre del fichero al que se enlaza
        ssize_t len;                      // Longitud del buffer
        char res_path[MAXPATH];           // Ruta absoluta del destino del enlace 

        if ( (len = readlink(path, buffer_link, sizeof(buffer_link) - 1)) == -1) {
            perror("Error al leer el destino del enlace simbolico");   

        } else {
            buffer_link[len] = '\0';      // readlink() no añade el '\0' al final del string !

            // Llamada realpath() para resolver la ruta canónica absoluta del destino del enlace:
            if (realpath(path, res_path) != NULL) {
                // Muestra la ruta absoluta del fichero destino
                printf(" -> %s\n", res_path);
            } else {
                // Si el enlace está roto (apunta a un archivo inexistente), mostramos lo que leímos
                printf(" -> %s\n", buffer_link);
            }
        }

    } else
        printf("\n");
}



/*** FUNCIONES RECURSIVAS ***/

// Borrado recursivo:
void delRec(char * arg){

    if (!EsDirectorio(arg)) {           // NO es un directorio (Caso base)
        if (unlink(arg) == -1){         // Error de borrado: Aunque el fichero exista, puede dar error la llamada unlink().
        char mensaje_error[MAXMERROR];
        sprintf(mensaje_error, "Imposible borrar %s", arg);
        perror(mensaje_error);
        }

    }else {                            // ES UN DIRECTORIO
        DIR *direcc = opendir(arg);
        if (direcc == NULL){           // Si no es capaz de abrir la ruta del directorio:
            char mensaje_error[MAXMERROR];
            sprintf(mensaje_error, "Imposible borrar %s", arg);
            perror(mensaje_error);
            return;
        }
                
        // Recorrido del contenido del directorio:
        struct dirent *cont;
        while ((cont = readdir(direcc)) != NULL){     // readdir() lee el elemento actual del contenido de direcc y pasa al siguiente
            // Omitimos el directorio actual y el padre:
            if (!strcmp(cont->d_name, ".") || !strcmp(cont->d_name, ".."))
                continue;
            //Ponemos la ruta completa:
            char ruta[MAXPATH];
            sprintf(ruta, "%s/%s", arg, cont->d_name);
            delRec(ruta);                         // (Caso recursivo)
        }

        // Cerramos la ruta del directorio:
        closedir(direcc);

        // Borramos el directorio ahora vacio:
        if (rmdir(arg) == -1){                        // Error de borrado del directorio:
            char mensaje_error[MAXMERROR];
            sprintf(mensaje_error, "Imposible borrar %s", arg);
            perror(mensaje_error);     
        }
    }
}

// Listado recursivo:
void listarRec(char *path, int reca, int recb, int hid,
                      int lon, int link, int acc){
            
    DIR *direcc = opendir(path);         // Si no es capaz de abrir la ruta del directorio:
    if (direcc == NULL) {
        char mensaje_error[MAXMERROR];
        sprintf(mensaje_error, "Imposible abrir directorio %s", path);
        perror(mensaje_error);
        return;
    }


    // Con recb, se hace recursion ANTES de la impresion del directorio actual:
    if (recb) {
        // Nos aseguramos de no hacer los dos recorridos recursivos:
        reca = 0;

        // Recorrido y acceso del contenido del directorio:
        struct dirent *entry;
        while ((entry = readdir(direcc)) != NULL) {

            // Se omiten (o no, segun hid) los directorios actual y padre:
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;
            if (!hid && entry->d_name[0] == '.')
                continue;

            // Aniade el contenido a la ruta:
            char subruta[MAXPATH];
            snprintf(subruta, sizeof(subruta), "%s/%s", path, entry->d_name);

            struct stat st;
            if (!lstat(subruta, &st) && S_ISDIR(st.st_mode))                   // Caso recursivo
                listarRec(subruta, reca, recb, hid, lon, link, acc);

        }

        rewinddir(direcc); // Reiniciamos el puntero del directorio (iterador) para imprimir el contenido del actual
    }

    // IMPRESION DEL DIRECTORIO ACTUAL:

    // Redefinimos path como la ruta absoluta (por si acaso no lo es):
    char res_path[MAXPATH];                 // Ruta absoluta del destino del enlace
    if (realpath(path, res_path) == NULL){
        char mensaje_error[MAXMERROR];
        sprintf(mensaje_error, "Error al resolver la ruta absoluta para %s", path);
        perror(mensaje_error);
        return;
    }

    // Impresion directorio actual:
    printf("************%s\n", res_path);

    struct dirent *entry;
    while ((entry = readdir(direcc)) != NULL) {      // Volvemos a recorrer el contenido (iterador reiniciado si recb)
        
        // Se omiten (o no, segun hid) los directorios actual y padre:
        if (!hid && entry->d_name[0] == '.')
            continue;

        // Aniade el contenido a la ruta:
        char ruta[MAXPATH + 256];
        sprintf(ruta, "%s/%s", res_path, entry->d_name);

        struct stat st;
        if (lstat(ruta, &st) == -1) {
            char mensaje_error[MAXMERROR];
            sprintf(mensaje_error, "***Error al acceder a %s", path);
            perror(mensaje_error);
            continue;
        }

        // Impresion:
        imprimirElementoList(entry->d_name, ruta, &st, lon, link, acc);
    }

    // Con reca, se hace recursion DESPUES de la impresion del directorio actual:
    if (reca) {
        rewinddir(direcc);               // Como es despues de la impresion, se reinicia el puntero al empezar

        // Recorrido y acceso del contenido del directorio:
        while ((entry = readdir(direcc)) != NULL) {

            // Se omiten (o no, segun hid) los directorios actual y padre:
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;
            if (!hid && entry->d_name[0] == '.')
                continue;

            // Aniade el contenido a la ruta:
            char subruta[MAXPATH + 256];
            sprintf(subruta, "%s/%s", res_path, entry->d_name);

            struct stat st;
            if (lstat(subruta, &st) == 0 && S_ISDIR(st.st_mode)) {          // Caso recursivo
                listarRec(subruta, reca, recb, hid, lon, link, acc);
            }
        }
    }

    // Cerramos la ruta del directorio:
    closedir(direcc);

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
                if (getcwd(cad, sizeof(cad)) == NULL){           //Asignamos cad a la ruta de dierectorio completa
                    perror("Error al obtener el directorio");
                    return;
                }
                break;

            case 'D':
                char dir_actual[MAXPATH];
                if (getcwd(dir_actual, sizeof(dir_actual)) == NULL) {
                    perror("Error al obtener el directorio");
                    return;
                }
                
                size_t len_home = strlen(pw->pw_dir);
                // Si el directorio actual empieza por la ruta de pw_dir (/home/usuario)
                if (strncmp(dir_actual, pw->pw_dir, len_home) == 0) {
                    snprintf(cad, sizeof(cad), "~%s", dir_actual + len_home);    // Contraemos la ruta
                } else {
                    strncpy(cad, dir_actual, sizeof(cad) - 1);
                }
                break;


            case 'B':
                char cad2[MAXPATH];    // Almacenamos en otro string el directorio actual sin el "/"
                if (getcwd(cad, sizeof(cad)) == NULL){
                    perror("Error al obtener el directorio");
                    return;
                }
                strcpy(cad2, cad);                         
                strcpy(cad, strrchr(cad2, '/') + 1);       // Redefinimos cad
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
    // Liberamos la memoria reservada por las listas:
    VaciarTablaFicherosAbiertos();
    PathClear();

    exit(0);
}

void Cmd_autores(char *arg[]) //si arg[0] == NULL, solo se escribe autores
{
    if(arg[0] == NULL){ //si solo se pasa autores
        printf ("diego lopez lopez: diego.lopez.lopez1\nbruno garcia fariña: bruno.gfarina\npablo muradas santa maria: pablo.muradas\n");
    }
    else if (!strcmp(arg[0], "-l")){
        printf ("diego.lopez.lopez1\nbruno.gfarina\npablo.muradas\n");
    }
    else if (!strcmp(arg[0], "-n")){
        printf ("diego lopez lopez\nbruno garcia fariña\npablo muradas santa maria\n");
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
    
    // Llamada al sistema:
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

    // Comprobamos si el descriptor existe en nuestra lista interna:
    if (NombreFicheroDescriptor(df) == NULL){
        printf("Imposible cerrar descriptor %d: Descriptor no valido\n", df);
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
        char mensaje_error[MAXMERROR];
        sprintf(mensaje_error, "Error al intentar posicionar el descriptor %d en el offset %ld", df, pos);
        perror(mensaje_error);
    }else 
        printf("Descriptor %d posicionado en %ld\n", df, pos);


}


void Cmd_readstr (char *args[]){
    
    // No se pasan 2 argumentos como minimo:
    if(args[0] == NULL || args[1] == NULL){
        printf("Parametros incorrectos\n");
        return;
    }

    int df, cont;
    ssize_t leidos;
    char *buffer;

    // Guardamos en variables los argumentos del descriptor y contenido:
    df = atoi(args[0]);
    cont = atoi(args[1]);
    
    if (cont < 0) {
        printf("Parametros incorrectos\n");
        return;
    }

    buffer = malloc(cont+1); //reservamos +1 de memoria para '\0'


    if(buffer == NULL){
        printf("Imposible asignar memoria para la lectura\n");
        return;
    }

    // Hacemos la llamada al sistema read():
    leidos = read(df, buffer, cont);

    if(leidos == -1){
        char mensaje_error[MAXMERROR];
        sprintf(mensaje_error,"Error al intentar leer el descriptor %d", df);
        perror(mensaje_error);

    }else{
        buffer[leidos] = '\0';  // Aniadimos el '\0'
        printf("Leidos %zd bytes del descriptor %d\n%s\n", leidos, df, buffer);    
    }

    // Liberamos la memoria que habiamos reservado:
    free(buffer);
}


void Cmd_writestr (char *args[]){

    if(args[0] == NULL || args[1] == NULL){
        printf("Parametros incorrectos\n");
        return;
    }

    int df = atoi(args[0]);
    ssize_t escritos;
    size_t total_len = 0;
    char *buffer;
    int i;

    // Recorremos los argumentos para contar longitud:
    for (i = 1; args[i] != NULL; i++) {
        total_len += strlen(args[i]);
        if (args[i+1] != NULL) {
            total_len++; // Añadimos 1 para el espacio separador
        }
    }

    // Reserva de memoria:
    buffer = malloc(total_len + 1); // +1 para el '\0'
    if (buffer == NULL) {
        return;
    }

    // Construimos la frase completa
    strcpy(buffer, args[1]);     // Primera palabra
    for (i = 2; args[i] != NULL; i++) {
        strcat(buffer, " ");     // Añadimos el espacio
        strcat(buffer, args[i]); // Añadimos la siguiente palabra
    }

    // Llamada al sistema:
    escritos = write(df, buffer, total_len);

    // Salida:
    if (escritos == -1) {
        char mensaje_error[MAXMERROR];
        sprintf(mensaje_error, "Error intentar escribir %zu bytes en el descriptor %d", total_len, df);
        perror(mensaje_error);

    } else {
        printf("Escritos %zd bytes en el descriptor %d\n", escritos, df);
    }

    // Liberamos la memoria reservada:
    free(buffer);
}


void Cmd_makefile (char *args[]){

    // Se pasa solo "makefile", se muestra el cwd:
    if((args[0] == NULL)){
        MostrarDirActual();
        return;
    }

    int df;

    // Hacemos la llamada open() con flags O_CREAT, O_WRONLY, O_EXCL:
    if ((df = open(args[0], O_CREAT | O_WRONLY | O_EXCL, 0777)) == -1){
        char mensaje_error[MAXMERROR];
        sprintf(mensaje_error, "Imposible crear %s", args[0]); 
        perror(mensaje_error);
        return;
    }

    // Y lo cerramos (solo se crea):
    close(df);
}


void Cmd_makedir (char *args[]){

    // Se pasa solo "makedir", se muestra el cwd:
    if((args[0] == NULL)){
        MostrarDirActual();
        return;
    }

    int df;

    // Llamada al sistema mkdir():
    if ((df = mkdir(args[0], 0777)) == -1) {     // Aniadimos permisos de ejecucion porque es un directorio y se deberia
        char mensaje_error[MAXMERROR];           // poder acceder a el
        sprintf(mensaje_error, "Imposible crear %s", args[0]); 
        perror(mensaje_error);
        return;
    }

}


void Cmd_delete (char *args[]){

    // Se pasa solo "delete", se muestra el cwd:
    if (args[0] == NULL){
        MostrarDirActual();
        return;
    }

    // Recorre los ficheros/directorios pasados:
    int i;

    for (i = 0; args[i] != NULL; i++){

        // Si es un directorio:
        if (EsDirectorio(args[i])){
            if (rmdir(args[i]) == -1){          // Si no esta vacio / error de borrado:
                char mensaje_error[MAXMERROR];
                sprintf(mensaje_error, "Imposible borrar %s", args[i]);
                perror(mensaje_error);
                continue;
            }

        // Si es un fichero normal / enlace:
        } else {                           
            if (unlink(args[i]) == -1){         // Error de borrado: Aunque el fichero exista, puede dar error la llamada unlink().
                char mensaje_error[MAXMERROR];
                sprintf(mensaje_error, "Imposible borrar %s", args[i]);
                perror(mensaje_error);
                continue;
            }

        }

    }

}

void Cmd_deltree (char *args[]){

    // Se pasa solo "deltree", se muestra el cwd:
    if (args[0] == NULL){
        MostrarDirActual();
        return;
    }

    // Recorre los ficheros/directorios pasados:
    int i;

    for (i = 0; args[i] != NULL; i++)
        delRec(args[i]);               // Llamamos a la funcion recursiva

}

void Cmd_listfile (char *args[]){

    // Si es un argumento especial:
    int i = 0;
    int longmode = 0, linkmode = 0, accmode = 0;
    while ((args[i] != NULL) && (args[i][0] == '-')){
        if (!strcmp(args[i], "-long"))     // creation (or access) time, number of links, inode, mode, owner, group.
                longmode = 1;
            if (!strcmp(args[i], "-link"))     // if file were to represent a symbolic link, the file it points to is also printed.
                linkmode = 1;
            if (!strcmp(args[i], "-acc"))      // last access time is used
                accmode = 1;
            
            i++;
    }        
    

    // Se pasa solo "listfile", muestra el cwd:
    if (args[i] == NULL){
        MostrarDirActual();
        return;
    }
    
    // Recorremos los ficheros pasados:
    while(args[i] != NULL){
        struct stat st;

        //Examina el fichero/directorio pasado:
        if (lstat(args[i], &st) == -1) {       // El elemento no existe o no hay permisos
            char mensaje_error[MAXMERROR];
            sprintf(mensaje_error, "***Error al acceder a %s", args[i]);
            perror(mensaje_error);

            i++;
            continue;
        }

        // PROCESO DE LISTAR:
        imprimirElementoList(args[i], args[i], &st, longmode, linkmode, accmode);
        
        i++;
    }
        
}


void Cmd_list(char *args[])
{
    int recamode = 0, recbmode = 0, hidmode = 0, longmode = 0, linkmode = 0, accmode = 0;
    int i = 0;

    // Si es un argumento "especial":
    while (args[i] != NULL && args[i][0] == '-') {

        if (strcmp(args[i], "-reca") == 0)
            recamode = 1;
        else if (strcmp(args[i], "-recb") == 0)
            recbmode = 1;
        else if (strcmp(args[i], "-hid") == 0)
            hidmode = 1;
        else if (strcmp(args[i], "-long") == 0)
            longmode = 1;
        else if (strcmp(args[i], "-link") == 0)
            linkmode = 1;
        else if (strcmp(args[i], "-acc") == 0)
            accmode = 1;

        i++;
    }

    // Se pasa solo "list", muestra el cwd:
    if (args[i] == NULL){
        MostrarDirActual();
        return;
    }

    // Recorremos los ficheros / directorios pasados:
    while (args[i] != NULL) {
        struct stat st;

        // Examinamos con lstat y comprobamos con S_ISDIR en vez de llamar a EsDirectorio()
        // porque necesitamos pasar st (si llamasemos a la funcion estamos haciendo lstat 2 veces):
        if (lstat(args[i], &st) == -1) {
            char mensaje_error[MAXMERROR];
            sprintf(mensaje_error, "***Error al acceder a %s", args[i]);
            perror(mensaje_error);
            i++;
            continue;
        }

        // Si es un directorio, se usa la funcion para listar recursivamente:
        if (S_ISDIR(st.st_mode))
            listarRec(args[i], recamode, recbmode, hidmode, longmode, linkmode, accmode);

        // Si no, se imprime el fichero (como en listfile):
        else 
            imprimirElementoList(args[i], args[i], &st, longmode, linkmode, accmode);

        i++;
    }
}


//Comandos que no están en la practica pero son imprescindibles para el funcionamiento:
void Cmd_exec (char *arg[])
{
  if (execv(Ejecutable(arg[0]),arg)==-1)
	perror ("Imposible ejecutar");
}

void Cmd_pplano (char *arg[])
{
  Proceso(arg,0);
}

void Cmd_importpath (char *arg[])
{
    PathAddPath();
}

