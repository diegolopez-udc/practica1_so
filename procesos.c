#include "procesos.h"

// Definimos el array de estructuras que relaciona nombre de la senal con su numero:
// NO MOVER A .H, QUE ES ESTÁTICA !! Se duplicaría en los archivos de los que depende el .h => PROVOCA WARNING
static struct SEN sigstrnum[] = {
    {"HUP", SIGHUP},
    {"INT", SIGINT},
    {"QUIT", SIGQUIT},
    {"ILL", SIGILL},
    {"TRAP", SIGTRAP},
    {"ABRT", SIGABRT},
    {"IOT", SIGIOT},
    {"BUS", SIGBUS},
    {"FPE", SIGFPE},
    {"KILL", SIGKILL},
    {"USR1", SIGUSR1},
    {"SEGV", SIGSEGV},
    {"USR2", SIGUSR2},
    {"PIPE", SIGPIPE},
    {"ALRM", SIGALRM},
    {"TERM", SIGTERM},
    {"CHLD", SIGCHLD},
    {"CONT", SIGCONT},
    {"STOP", SIGSTOP},
    {"TSTP", SIGTSTP},
    {"TTIN", SIGTTIN},
    {"TTOU", SIGTTOU},
    {"URG", SIGURG},
    {"XCPU", SIGXCPU},
    {"XFSZ", SIGXFSZ},
    {"VTALRM", SIGVTALRM},
    {"PROF", SIGPROF},
    {"WINCH", SIGWINCH},
    {"IO", SIGIO},
    {"SYS", SIGSYS},

// Ahora, definimos las senales que no hay en todos los sistemas operativos:
#ifdef SIGPOLL
    {"POLL", SIGPOLL},
#endif
#ifdef SIGPWR
    {"PWR", SIGPWR},
#endif
#ifdef SIGEMT
    {"EMT", SIGEMT},
#endif
#ifdef SIGINFO
    {"INFO", SIGINFO},
#endif
#ifdef SIGSTKFLT
    {"STKFLT", SIGSTKFLT},
#endif
#ifdef SIGCLD
    {"CLD", SIGCLD},
#endif
#ifdef SIGLOST
    {"LOST", SIGLOST},
#endif
#ifdef SIGCANCEL
    {"CANCEL", SIGCANCEL},
#endif
#ifdef SIGTHAW
    {"THAW", SIGTHAW},
#endif
#ifdef SIGFREEZE
    {"FREEZE", SIGFREEZE},
#endif
#ifdef SIGLWP
    {"LWP", SIGLWP},
#endif
#ifdef SIGWAITING
    {"WAITING", SIGWAITING},
#endif
    {NULL, -1},

}; // Fin array sigstrnum

// Funciones para jobs y deljobs:

// Devuelve el numero de senal a partir del nombre:
int ValorSenal(char *sen)
{
    int i;

    for (i = 0; sigstrnum[i].nombre != NULL; i++)
    {
        if (!strcmp(sen, sigstrnum[i].nombre))
            return sigstrnum[i].senal;
    }
    return -1;
}

// Devuelve el nombre senal a partir de la senal para sitios donde no hay sig2str:
char *NombreSenal(int sen)
{
    int i;

    for (i = 0; sigstrnum[i].nombre != NULL; i++)
        if (sen == sigstrnum[i].senal)
            return sigstrnum[i].nombre;

    return ("SIGUNKNOWN");
}

// IMPLEMENTACION DE LA LISTA DE PROCESOS:

static LISTASIMPLE ListaProcesos;

/***************************************/
/*************OPERACIONES************************/

static void ActualizarEstadoProceso(tProceso *p)
{
    int wstatus;
    pid_t res;

    // Si termino previamente, no volvemos a llamar a waitpid (solo se informa una vez):
    if (p->state == FINISHED || p->state == SIGNALED)
        return;

    res = waitpid(p->pid, &wstatus, WNOHANG | WUNTRACED | WCONTINUED); // asíncrono

    // wstatus solo tiene contenido significativo si waitpid devuelve el pid del proceso:
    if (res == p->pid)
    {

        // Hijo acabo por si mismo:
        if (WIFEXITED(wstatus))
        {
            p->state = FINISHED;
            p->svalue = WEXITSTATUS(wstatus); // Extraemos su codigo de retorno

            // Hijo fue matado por una senal:
        }
        else if (WIFSIGNALED(wstatus))
        {
            p->state = SIGNALED;
            p->svalue = WTERMSIG(wstatus); // Extraemos el numero de senal

            // Hijo esta pausado:
        }
        else if (WIFSTOPPED(wstatus))
        {
            p->state = STOPPED;
            p->svalue = WSTOPSIG(wstatus); // Extraemos la senal

            // Hijo volvio a arrancar
        }
        else if (WIFCONTINUED(wstatus))
        {
            p->state = ACTIVE;
            p->svalue = 0; // Le asignamos el valor de ACTIVE
        }
    }
}

static void ImprimirProceso(void *el)
{
    tProceso *p = (tProceso *)el;
    char buffer[MAXDATE];
    char estado_str[64];
    struct tm *tm_info;
    int prio;

    // Actualizamos su estado antes de imprimir:
    ActualizarEstadoProceso(p);

    // Obtenemos su prioridad actual:
    if (p->state == FINISHED || p->state == SIGNALED)
        prio = -1;
    else
        prio = getpriority(PRIO_PROCESS, p->pid);

    // Formateamos fecha:
    tm_info = localtime(&p->date);
    if (tm_info != NULL)
    {
        strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", tm_info);
    }
    else
    {
        strcpy(buffer, "0000/00/00 00:00:00");
    }

    // Accedemos al uid efectivo del proceso:
    struct passwd *pw = getpwuid(p->uid);
    const char *user;
    if (pw != NULL)
        user = pw->pw_name;
    else
        user = "desconocido";

    // Formateamos estado y el valor de retorno o senal:
    switch (p->state)
    {
    case ACTIVE:
        snprintf(estado_str, sizeof(estado_str), "ACTIVO (000)");
        break;
    case FINISHED:
        snprintf(estado_str, sizeof(estado_str), "TERMINADO (%03d)", p->svalue);
        break;
    case SIGNALED:
        snprintf(estado_str, sizeof(estado_str), "SENALADO (%s)", NombreSenal(p->svalue));
        break;
    case STOPPED:
        snprintf(estado_str, sizeof(estado_str), "PARADO (%s)", NombreSenal(p->svalue));
        break;
    }

    // Impresion:
    printf("%6d       %s p=%d %s %s %s\n", p->pid, user, prio, buffer, estado_str, p->comando);
}

int AniadirProcesoBackground(pid_t pid, char *arg[])
{
    tProceso *p;    
    int i;
    size_t len = 0;

    if ((p = malloc(sizeof(tProceso))) == NULL)
        return -1;

    p->pid = pid;
    p->uid = geteuid();
    p->state = ACTIVE;
    p->svalue = 0;
    p->date = time(NULL);
    p->comando[0] = '\0';

    // Concatenamos comando del proceso:
    for (i = 0; arg[i] != NULL; i++)
    {
        size_t arg_len = strlen(arg[i]); // Tomamos la longitud del argumento
        size_t sep;                      // Indica si se necesita separacion

        if (i > 0)
            sep = 1;
        else
            sep = 0;

        if (len + arg_len + sep < MAXCOMMAND - 1)
        { // La longitud debe caber en el tamanho maximo para
            if (sep)
            {                            // el comando
                strcat(p->comando, " "); // Concatenamos separacion
                len++;
            }
            strcat(p->comando, arg[i]); // Concatenamos argumento
            len += arg_len;
        }
    }

    if (AniadirElemento(ListaProcesos, p) == -1)
    {
        free(p);
        return -1;
    }
    return 0;
}

void ListarProcesosBackground(void)
{
    ImprimirListaCompleta(ListaProcesos, 0, ImprimirProceso);
}

void BorrarProcesosBackground(int term, int sig)
{
    int i = 0;
    tProceso *p;

    // Recorremos la lista:
    while ((p = (tProceso *)GetElementoAtPos(ListaProcesos, i)) != NULL)
    {

        // Actualizamos estado por si alguno ha terminado:
        ActualizarEstadoProceso(p);

        // Eliminamos los procesos que hayan acabado de forma normal o por senal:
        if ((term && p->state == FINISHED) || (sig && p->state == SIGNALED))
            BorrarElementoAtPos(ListaProcesos, i); // No incrementamos i porque el siguiente estara ahora en i
        else
            i++;
    }
}


void VaciarListaProcesos(void)
{
    BorrarLista(ListaProcesos);
}
