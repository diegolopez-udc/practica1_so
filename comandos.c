#include "comandos.h"



/*********************************************************/
/***************GESTION DEL PATH / PROCESOS / FUNCIONES AUXILIARES***************/

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
        snprintf(mensaje_error, sizeof(mensaje_error), "Error al acceder a %s", dir);
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
        snprintf(mensaje_error ,sizeof(mensaje_error), "Imposible borrar %s", arg);
        perror(mensaje_error);
        }

    }else {                            // ES UN DIRECTORIO
        DIR *direcc = opendir(arg);
        if (direcc == NULL){           // Si no es capaz de abrir la ruta del directorio:
            char mensaje_error[MAXMERROR];
            snprintf(mensaje_error, sizeof(mensaje_error), "Imposible borrar %s", arg);
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
            snprintf(ruta, sizeof(ruta), "%s/%s", arg, cont->d_name);
            delRec(ruta);                         // (Caso recursivo)
        }

        // Cerramos la ruta del directorio:
        closedir(direcc);

        // Borramos el directorio ahora vacio:
        if (rmdir(arg) == -1){                        // Error de borrado del directorio:
            char mensaje_error[MAXMERROR];
            snprintf(mensaje_error, sizeof(mensaje_error), "Imposible borrar %s", arg);
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
        snprintf(mensaje_error, sizeof(mensaje_error), "Imposible abrir directorio %s", path);
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
        snprintf(mensaje_error, sizeof(mensaje_error), "Error al resolver la ruta absoluta para %s", path);
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
        snprintf(ruta, sizeof(ruta), "%s/%s", res_path, entry->d_name);

        struct stat st;
        if (lstat(ruta, &st) == -1) {
            char mensaje_error[MAXMERROR];
            snprintf(mensaje_error, sizeof(mensaje_error), "***Error al acceder a %s", path);
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
            snprintf(subruta, sizeof(subruta), "%s/%s", res_path, entry->d_name);

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


/********** COMANDOS PRACTICA 1 **********/
/***********************************************/


void Cmd_fin (char * arg[])  /*todos los cmd_ comparten prototipo*/
{                            /*reciben los mismos parametros aunque no los usen*/
    // Liberamos la memoria reservada por las listas:
    VaciarTablaFicherosAbiertos();
    PathClear();
    VaciarTablaMemoria();

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
        printf("Comandos de la Practica 2: allocate deallocate i-o read write memory recurse memfill memdump\n");

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
    int df = -1;
    int force = 0;
    int i = 0;
    
    // Recorrido de parametros:
    while(arg[i] != NULL){

        if(strcmp(arg[i], "-f") == 0)     // Con parametro -f:
            force = 1;

        else                              // Aqui asignamos el descriptor a df
            df=atoi(arg[i]);

        i++;
    }

    // Si no se paso ningun descriptor o es invalido:
    if (df < 0){
        ListarFicherosAbiertos(); 
        return;
    }

    // Comprobamos si el descriptor existe en nuestra lista interna:
    if (NombreFicheroDescriptor(df) == NULL){
        printf("Imposible cerrar descriptor %d: Descriptor no valido\n", df);
        return;
    }

    // Si corresponde a un fichero mapeado y no se ha pasado -f:
    if (!force && EsDescriptorMapeado(df)){
        printf("Descriptor %d corresponde a fichero mapeado. Usa -f para cerrarlo\n", df);
        return;
    }

    // Llamada al sistema close:
    if (close(df)==-1)
        perror("Imposible cerrar descriptor");
    
    // Eliminamos de la lista: 
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

    snprintf(aux, sizeof(aux), "duplicado de %d (%s)",df, p);
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
        snprintf(mensaje_error, sizeof(mensaje_error), "Error al intentar posicionar el descriptor %d en el offset %ld", df, pos);
        perror(mensaje_error);
    }else 
        printf("Descriptor %d posicionado en %ld\n", df, new_offset);


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
        snprintf(mensaje_error, sizeof(mensaje_error), "Error al intentar leer el descriptor %d", df);
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
        snprintf(mensaje_error, sizeof(mensaje_error), "Error intentar escribir %zu bytes en el descriptor %d", total_len, df);
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
        snprintf(mensaje_error, sizeof(mensaje_error), "Imposible crear %s", args[0]); 
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
        snprintf(mensaje_error, sizeof(mensaje_error), "Imposible crear %s", args[0]); 
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
                snprintf(mensaje_error, sizeof(mensaje_error), "Imposible borrar %s", args[i]);
                perror(mensaje_error);
                continue;
            }

        // Si es un fichero normal / enlace:
        } else {                           
            if (unlink(args[i]) == -1){         // Error de borrado: Aunque el fichero exista, puede dar error la llamada unlink().
                char mensaje_error[MAXMERROR];
                snprintf(mensaje_error, sizeof(mensaje_error), "Imposible borrar %s", args[i]);
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
            snprintf(mensaje_error, sizeof(mensaje_error), "***Error al acceder a %s", args[i]);
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
            snprintf(mensaje_error, sizeof(mensaje_error), "***Error al acceder a %s", args[i]);
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


/********** COMANDOS PRACTICA 2 **********/
/***********************************************/


// Conversion de cadena a puntero de void:
void * CadenatoPointer (char * s)
{
  void *p;
  sscanf(s,"%p",&p);
  return p;
}


// Funcion auxiliar para allocate -malloc:
void do_AllocateMalloc(char *tr[]){
    size_t tam;
    void *p;

    // No se le pasa size:
    if (tr[0] == NULL){
        ImprimirTablaMemoriaFiltrada(0, 0);  // Imprimimos bloques asignados para malloc
        return;
    }

    // Obtenemos el tamanho del bloque de forma numerica:
    tam = (size_t) strtoul(tr[0], NULL, 10);

    // No se pueden crear bloques de 0 Bytes:
    if (tam == 0){
        printf("No se asignan bloques de 0 bytes\n");
        return;
    }

    // Hacemos la llamada al sistema malloc():
    if((p = malloc(tam)) == NULL){
        perror("Imposible obtener memoria con malloc");
        return;
    }

    // Reserva de memoria exitosa:
    InsertarBloqueMalloc(p, tam);
    printf ("Asignados %lu bytes en %p\n",(unsigned long) tam, p);
}


// Funcion auxiliar para allocate -createshared / -shared:
void * ObtenerMemoriaShmget (key_t clave, size_t tam){
    void * p;
    int aux,id,flags = 0777;                      // Los 9 bits menos significativos de los flags:permisos
    struct shmid_ds s;

    if (tam)                                      // IMPORTANTE: tam distito de 0 indica crear
        flags = flags | IPC_CREAT | IPC_EXCL;     // Cuando no es crear pasamos de tamano 0
    if (clave == IPC_PRIVATE){                    // No nos vale -> error
        errno = EINVAL; 
        return NULL;
    }
    if ((id = shmget(clave, tam, flags)) == -1)   // Llamada al sistema para acceder
        return (NULL);                            // a la memoria compartida
    if ((p = shmat(id, NULL, 0)) == (void*) -1){  // Adjunta la memoria compartida al espacio de direcciones 
        aux=errno;                                // de la shell
        if(tam)
             shmctl(id, IPC_RMID, NULL);          // Borra el segmento de memoria compartida
        errno=aux;
        return (NULL);
    }
    shmctl (id, IPC_STAT, &s);                    // Si no es crear, necesitamos el tamano, que es s.shm_segsz

    // Guardar en la lista
    InsertarBloqueShared(p, tam, clave);
    return (p);
}

// Funcion auxiliar para allocate -createshared:
void do_AllocateCreateShared (char *tr[]){
    key_t cl;
    size_t tam;
    void *p;

    if (tr[0]==NULL || tr[1]==NULL) {
		ImprimirTablaMemoriaFiltrada(1, 0);  // Imprimimos la lista solo para los bloques de memoria compartida
		return;
    }
  
    // Obtenemos la clave y el tamanho en formato nummerico:
    cl = (key_t) strtoul(tr[0], NULL, 10);
    tam = (size_t) strtoul(tr[1], NULL, 10);
    
    // No se pueden asignar bloques de 0 Bytes:
    if (tam == 0) {
	    printf ("No se asignan bloques de 0 bytes\n");
	    return;
    }

    // Hacemos uso de nuestra funcion auxiliar:
    if ((p = ObtenerMemoriaShmget(cl,tam))!=NULL)
		printf ("Asignados %lu bytes en %p\n",(unsigned long) tam, p);
    else
		printf ("Imposible asignar memoria compartida clave %lu:%s\n",(unsigned long) cl,strerror(errno));

}

// Funcion auxiliar para allocate -shared:
void do_AllocateShared (char *tr[]){
    key_t cl;
    void *p;

    if (tr[0]==NULL) {
	    ImprimirTablaMemoriaFiltrada(1, 0);  // Imprimimos la lista solo para los bloques de memoria compartida
		return;
    }
  
    // Obtenemos la clave en formato nummerico:
    cl = (key_t) strtoul(tr[0],NULL,10);

    // Llamamos a la funcion auxiliar previa:
    if ((p = ObtenerMemoriaShmget(cl,0)) != NULL)
		printf ("Asignada memoria compartida de clave %lu en %p\n",(unsigned long) cl, p);
    else
		printf ("Imposible asignar memoria compartida clave %lu: %s\n",(unsigned long) cl, strerror(errno));
}

// Funcion de ayuda para do_AllocateMap():
void * MapearFichero (char * fichero, int protection){
    int df, map = MAP_PRIVATE, modo = O_RDONLY;
    struct stat s;
    void *p;

    if (protection & PROT_WRITE)
          modo = O_RDWR;
    if (stat(fichero, &s) == -1 || (df = open(fichero, modo)) == -1)
          return NULL;
    if ((p = mmap(NULL, s.st_size, protection, map, df, 0)) == MAP_FAILED)
           return NULL;

    // Insertamos el bloque de memoria y el fichero que hemos abierto:
    InsertarBloqueMmap(p, s.st_size, df, fichero);
    AniadirFicheroAbierto(df, fichero, modo);
    return p;
}


// Funcion auxiliar para allocate -mmap:
void do_AllocateMmap(char *arg[]){ 
    char *perm;
    void *p;
    int protection=0;
     
    if (arg[0]==NULL){
        ImprimirTablaMemoriaFiltrada(2, 0);   // Imprimimos la lista solo para los bloques de mapeo de ficheros
        return;
    
    }if ((perm = arg[1]) != NULL && strlen(perm) < 4) {
            if (strchr(perm,'r')!=NULL) protection |= PROT_READ;
            if (strchr(perm,'w')!=NULL) protection |= PROT_WRITE;
            if (strchr(perm,'x')!=NULL) protection |= PROT_EXEC;
    }

    // Llamamos a la funcion auxiliar previa:
    if ((p = MapearFichero(arg[0], protection))==NULL){
            perror ("Imposible mapear fichero");
            return;
    }
    
    // Salida exitosa:
    printf ("fichero %s mapeado en %p\n", arg[0], p);
}

void Cmd_allocate(char *args[]){

    // Se pasa solo "allocate", se muestra TODA la lista de bloques de memoria:
    if (args[0] == NULL){
        ImprimirTablaMemoriaFiltrada(0, 1);
        return;
    }

    // Si no se le pasa un argumento "especial":
    if ((char) args[0][0] != '-'){
        printf("uso: allocate [-malloc|-shared|-createshared|-mmap] ....\n");
        return;
    }

    // Vemos de que tipo de allocate se trata:

    // Tipo malloc:
    if (!strcmp(args[0], "-malloc")){
        do_AllocateMalloc(args + 1);
    }

    // Tipo createshared:
    if (!strcmp(args[0], "-createshared")){
        do_AllocateCreateShared(args + 1);
    }

    // Tipo shared:
    if (!strcmp(args[0], "-shared")){
        do_AllocateShared(args + 1);
    }
    
    // Tipo mmap:
    if (!strcmp(args[0], "-mmap")){
        do_AllocateMmap(args + 1);
    }

}


// Funcion auxiliar para deallocate -shared:
/*
void do_DeallocateShared (key_t cl){
    
    void *p;                                      // Llamo a la funcion de la lista
                                                  // que devuelve la direccion donde
    if ((p=DireccionNodoShared (&L,cl))==NULL){   // Esta la memoria de clave cl
                printf ("No hay bloque de esa clave mapeado en el proceso\n");
                return;
    }
    shmdt(p);
    if (EliminarNodoDireccion (&L,p)==-1)  // Elimino el nodo
        printf ("Imposible quitar de lista: %s\n",strerror(errno));
}
*/


// Funcion auxiliar para deallocate -delkey:
/*
void do_SharedDelkey (char *args[]){
   
    key_t clave;
    int id;
    char *key=args[0];

    if (key==NULL || (clave=(key_t) strtoul(key,NULL,10))==IPC_PRIVATE){
        printf ("      delkey necesita clave_valida\n");
        return;
    }
    if ((id=shmget(clave,0,0666))==-1){
        perror ("shmget: imposible obtener memoria compartida");
        return;
    }
    if (shmctl(id,IPC_RMID,NULL)==-1)
        perror ("shmctl: imposible eliminar memoria compartida\n");
}
*/


void Cmd_deallocate(char *args[]){

}


// Funcion auxiliar para i-o y read:
/*
ssize_t LeerFichero (char *f, void *p, size_t cont){
   
    struct stat s;
    ssize_t  n;  
    int df,aux;

    if (stat (f,&s)==-1 || (df=open(f,O_RDONLY))==-1)
	    return -1;     
    if (cont==-1)   // Si pasamos -1 como bytes a leer lo leemos entero
	    cont=s.st_size;
    if ((n=read(df,p,cont))==-1){
	    aux=errno;
	    close(df);
	    errno=aux;
	    return -1;
    }
    close (df);
    return n;
}
*/


// Funcion auxiliar para i-o:
/*
void do_IOReadFile (char *ar[]){

    void *p;
    size_t cont=-1;  // Si no pasamos tamano se lee entero
    ssize_t n;
    if (ar[0]==NULL || ar[1]==NULL){
	    printf ("faltan parametros\n");
	    return;
    }
    p=CadenatoPointer(ar[1]);  // Convertimos de cadena a puntero
    if (ar[2]!=NULL)
	    cont=(size_t) atoll(ar[2]);

    if ((n=LeerFichero(ar[0],p,cont))==-1)
	    perror ("Imposible leer fichero");
    else
	    printf ("leidos %lld bytes de %s en %p\n",(long long) n,ar[0],p);
}
*/


void Cmd_io(char *args[]){

}


// Funcion auxiliar para memory:
/*
void Do_pmap (void) // Sin argumentos
 { pid_t pid;       // Hace el pmap (o equivalente) del proceso actual
   char elpid[32];
   char *argv[4]={"pmap",elpid,NULL};
   
   sprintf (elpid,"%d", (int) getpid());
   if ((pid=fork())==-1){
      perror ("Imposible crear proceso");
      return;
      }
   if (pid==0){
      if (execvp(argv[0],argv)==-1)
         perror("cannot execute pmap (linux, solaris)");
         
      argv[0]="procstat"; argv[1]="vm"; argv[2]=elpid; argv[3]=NULL;   
      if (execvp(argv[0],argv)==-1)        // No hay pmap, probamos procstat FreeBSD 
         perror("cannot execute procstat (FreeBSD)");
         
      argv[0]="procmap",argv[1]=elpid;argv[2]=NULL;    
            if (execvp(argv[0],argv)==-1)  // Probamos procmap OpenBSD
         perror("cannot execute procmap (OpenBSD)");
         
      argv[0]="vmmap"; argv[1]="-interleave"; argv[2]=elpid;argv[3]=NULL;
      if (execvp(argv[0],argv)==-1)        // Probamos vmmap Mac-OS
         perror("cannot execute vmmap (Mac-OS)");      
      exit(1);
  }
  waitpid (pid,NULL,0);
}
*/



void Cmd_memory(char *args[]){

}


///////////////////////////////////////////////////////////////////////////////

void Cmd_read(char *args[]){

    // Comprobamos que los parametros no son nulos:
    if (args[0] == NULL || args[1] == NULL || args[2] == NULL) {
        printf("Parametros incorrectos");
        return;   
    }

    int df = atoi(args[0]);
    int cont = atoi(args[2]);
    void *p = CadenatoPointer(args[1]);

    ssize_t leidos = read(df, p, cont);
    if (leidos == -1) {         // Como perror() solo acepta una cadena de texto, tenemos que pasarle el mensaje ya hecho:
        char mensaje_error[200];
        snprintf(mensaje_error, sizeof(mensaje_error), "Error al leer del descriptor %d a la direccion %p", df, p);
        perror(mensaje_error);
    } else {
        printf ("Leidos %zd bytes del descriptor %d en la direccion %p\n", leidos, df, p);
    }
}



void Cmd_write(char *args[]){

    // Comprobamos que los parametros son correctos:
    if (args[0] == NULL || args[1] == NULL || args[2] == NULL) {
        printf("Parametros incorrectos");
        return;
    }

    int df = atoi(args[0]);
    int cont = atoi(args[2]);
    void *p = CadenatoPointer(args[1]);

    ssize_t escritos = write(df, p, cont);
    if (escritos == -1) {
        char mensaje_error[200];
        snprintf(mensaje_error, sizeof(mensaje_error), "Error al escribir en el descriptor %d desde la direccion %p", df, p);
        perror(mensaje_error);
    }
    else {
        printf("Escritos %zd bytes en el descriptor %d desde la direccion %p", escritos, df, p);
    }

}


// Funcion auxiliar para recurse:

void Recursiva (int n){
    char automatico[MAXSTACK];
    static char estatico[MAXSTACK];

    printf ("parametro:%3d(%p) array %p, arr estatico %p\n",n,&n,automatico, estatico);

    if (n>0)
        Recursiva(n-1);
}

void Cmd_recurse(char *args[]){
    // Si no recibe argumento, no hacemos nada:
    if (args[0] == NULL) return;

    // Convertimos el texto a entero:
    int n = atoi(args[0]);

    // Llamamos recursivamente a la funcion auxiliar:
    Recursiva(n);
}

////////////////////////////////////////////////////////////////////////////

// Funcion auxiliar:
/*
void LlenarMemoria (void *p, size_t cont, unsigned char byte){
    unsigned char *arr=(unsigned char *) p;
    size_t i;

    for (i=0; i<cont;i++)
	    arr[i]=byte;
}
*/


void Cmd_memfill(char *args[]){

}


void Cmd_memdump(char *args[]){

}


/**********************************************/

//Comandos que no estan en la practica pero son imprescindibles para el funcionamiento:
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

