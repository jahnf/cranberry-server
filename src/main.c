/**
 * @file main.c
 * @author Jahn Fuchs
 *
 * mws - mini web share
 * A simple - single file/executable - webserver, with functionality to share
 * directories easily over a web interface. Easy to start with almost no setup.
 *
 */

/* first some windows specific includes and defines */
#ifdef _WIN32
    #include <windows.h>

    #define chdir(s) _chdir(s)
    #define APP_BASENAME "strawberry-server"

    #define MAIN_RETURN_TYPE VOID WINAPI
    #define ARGC_TYPE DWORD
    #define ARGV_TYPE LPTSTR*
#else
    /* unix/posix systems */
    #include <libgen.h>

    /* [ -- aix needs this */
    #include <sys/socket.h>
    #include <netinet/in.h>
    /* ] */
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>

    #define closesocket(s) close(s);
    #define APP_BASENAME basename(argv[0])
    #define inet_ntop(a,b,c,d,e) inet_ntop(a,b,c,d)

    #define MAIN_RETURN_TYPE void
    #define ARGC_TYPE int
    #define ARGV_TYPE char**
#endif

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

#include "ip_socket_utils.h"
#include "webthread.h"
#include "cmdline.h"
#include "settings.h"
#include "server_commands.h"
#include "websession.h"
#if LUA_SUPPORT
    #include "luasp.h"
#endif

#include "log.h"
SETLOGMODULENAME("main");

#define THREAD_STACK_SIZE 512000
#define DEFAULT_CONFIG_FILE "cranberry-server.ini"

/* Local globals */
static int main_exit_code = EXIT_SUCCESS; /* Return value of main program */
static int main_loop      =  1;           /* The main loop loops while main_loop is 1*/
static int listenfd4      = -1;           /* Listen file descriptor for ipv4 */
static int listenfd6      = -1;           /* Listen file descriptor for ipv6 */
static int exit_sig_caught=  0;           /* Set to 1 if a signal was caught */

static void server_deinitialize( thread_arg_t *args );

/* exit signal handler */
void exit_signal_handler( int sig )
{
    printf("\n--\nSignal %d caught...\n", sig);
    main_loop = 0;
    exit_sig_caught = 1;
    if( listenfd4 != -1 )
        closesocket(listenfd4);
    if( listenfd6 != -1 )
        closesocket(listenfd6);
}

/* Web server main */
MAIN_RETURN_TYPE server_main(ARGC_TYPE argc, ARGV_TYPE argv) 
{
    int IPv4 = 0;
    int IPv6 = 0;
    int highfd;

    size_t hit = 0;             /* counter for number of hits */
    char *config_file = NULL;   /* ini config file to load */

    fd_set addr_set;

    thread_arg_t *arguments, baseargs = {0};
    server_settings_t* pSettings = NULL;

    /* static = initialized to zeros */
    static struct sockaddr_in cli_addr4 =   {0};
    static struct sockaddr_in6 cli_addr6 =  {0};
    static struct sockaddr_in serv_addr4 =  {0};
    static struct sockaddr_in6 serv_addr6 = {0};
    
    socklen_t length;
    c_thread mythread;      /* Posix/Windows thread */

    #ifdef _WIN32
        /* Initialization of sockets for MS Windows. */
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(1,1), &wsa_data);
    #endif

    /* Initialize server settings */
    pSettings = baseargs.pSettings = settings_init();
    if( !pSettings ) {
        fprintf( stderr, "Error: settings_init\n" );
        main_exit_code = EXIT_FAILURE;
        goto label_nolog_exit;
    }

    {   /* Parse command line arguments */
        int err = cmdline_parse( &baseargs, argc, argv, &config_file );
        if( err != CMDLINE_OKAY ) {

            if( err == CMDLINE_HELP_REQUESTED )
                cmdline_print_help( APP_BASENAME, 1 );
            else
                cmdline_print_help( APP_BASENAME, 0 );

            if( err != CMDLINE_HELP_REQUESTED ) main_exit_code = EXIT_FAILURE;
            goto label_nolog_exit;
        }
    }

    /* load settings from config file */
    /* TODO settings_loadini needs more return values, for different errors.. */
    if( !config_file ) settings_loadini( pSettings, DEFAULT_CONFIG_FILE, 1 );
    else if( !settings_loadini( pSettings, config_file, 0) ) {
        fprintf( stderr, "error while reading config file '%s'\n", config_file );
        main_exit_code = EXIT_FAILURE;
        goto label_nolog_exit;
    }

    /* initialize logging ---------------------------------------------------- */
    if( log_init( pSettings->logfile, pSettings->loglevel_file, pSettings->loglevel_console ) != 0 ) {
        fprintf( stderr, "error: log_init\n" );
        main_exit_code = EXIT_FAILURE;
        goto label_nolog_exit;
    }
    /* ... log initialized, now we can use LOG, LOG_PRINT and PRINT macros */

    /* TODO i18n for webserver, for web interface we can use a lua based solution */

    /* try to change directory to servers wwwroot, if set... */
    if( pSettings->wwwroot && chdir(pSettings->wwwroot) == -1 ) {
        LOG( log_ERROR, "Cannot change to www root directory (%s), exiting...", pSettings->wwwroot);
        main_exit_code = EXIT_FAILURE;
        goto label_exit;
    } else {
        LOG(log_INFO, "no www root directory set, starting without.");
    }

    LOG( log_ALWAYS, "starting web server on port %d...", pSettings->port );

    /* register signal handler... */
    #ifdef _WIN32
        signal(SIGABRT, &sighandler);
        signal(SIGTERM, &sighandler);
        signal(SIGINT,  &sighandler);
    #else
    {
        struct sigaction action;
        sigemptyset( &action.sa_mask );
        action.sa_handler = exit_signal_handler;
        action.sa_flags = 0;
        if( sigaction( SIGABRT, &action, NULL ) < 0 ) /*signal install error*/ (void)NULL;
        if( sigaction( SIGTERM, &action, NULL ) < 0 ) /*signal install error*/ (void)NULL;
        if( sigaction( SIGINT,  &action, NULL ) < 0 ) /*signal install error*/ (void)NULL;
        action.sa_handler = SIG_IGN;
        if( sigaction( SIGPIPE, &action, NULL ) < 0 ) /*signal install error*/ (void)NULL;
    }
    #endif

    /* Check if port is in valid range */
    if( pSettings->port < SERVER_PORT_MIN || pSettings->port > SERVER_PORT_MAX ) {
        LOG( log_ERROR, "port error: port must be from %u-%u (is %d)",
                SERVER_PORT_MIN, SERVER_PORT_MAX, pSettings->port );
        main_exit_code = EXIT_FAILURE;
        goto label_exit;
    }

    /* Get address informations */
    if( !server_getaddrinfo( &baseargs, &IPv4, &serv_addr4, &IPv6, &serv_addr6 ) ) {
        LOG( log_ERROR, "getaddrinfo error: no addrinfo found.");
        main_exit_code = EXIT_FAILURE;
        goto label_exit;
    }

    /* Setup the network socket(s) */
    /* IPv6 */
    if( IPv6 ) {
        if( (listenfd6 = (int)socket(AF_INET6, SOCK_STREAM, 0)) < 0 ) {
            LOG( log_ERROR, "socket creation error (IPv6)");
            IPv6 = 0;
        } else {
            int vTrue = 1;
            setsockopt( listenfd6, SOL_SOCKET, SO_REUSEADDR, (char*)&vTrue, sizeof(int) );
        }
    }
    /* IPv4 */
    if( IPv4 ) {
        if( (listenfd4 = (int)socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
            LOG( log_ERROR, "socket creation error (IPv4)");
            IPv4 = 0;
        } else {
            int vTrue = 1;
            setsockopt( listenfd4, SOL_SOCKET, SO_REUSEADDR, (char*)&vTrue, sizeof(int) );
        }
    }

    /* Bind the listeners */
    if( IPv6 && bind(listenfd6, (struct sockaddr *)&serv_addr6,sizeof(struct sockaddr_in6)) < 0 ) {
        LOG( log_ERROR, "bind error (IPv6): %s", strerror(errno) );
        IPv6 = 0;
    }
    if( IPv4 && bind(listenfd4, (struct sockaddr *)&serv_addr4,sizeof(serv_addr4)) < 0 ) {
        if( !IPv6 )
            LOG( log_ERROR, "bind error (IPv4): %s", strerror(errno) );
        IPv4 = 0;
    }

    if( IPv6 && listen(listenfd6,64) < 0 ) IPv6 = 0;
    if( IPv4 && listen(listenfd4,64) < 0 ) IPv4 = 0;

    if( !IPv4 && !IPv6 ) {
        LOG_FILE( log_ERROR, "socket error: could not set up IPv4 or IPv6 socket." );
        main_exit_code = EXIT_FAILURE;
        goto label_exit;
    }

    /* Initialize server modules */
    LOG( log_INFO, "initializing server modules...");

    if( !( (baseargs.pDataSrvCmds = server_commands_init())
        && (baseargs.pDataSrvThreads = webthread_init())
        && (baseargs.pDataSrvSessions = websession_init( &baseargs )) ))
    {
        LOG( log_ERROR, "Initialization error." );
        main_exit_code = EXIT_FAILURE;
        goto label_exit;
    }

    #if LUA_SUPPORT
    if( pSettings->scripting.enabled  && !(baseargs.pDataLuaScripting
                                           = luasp_init( &baseargs )) ) {
        LOG( log_ERROR, "scripting initialization error." );
        main_exit_code = EXIT_FAILURE;
        goto label_exit;
    }
    #endif

    #ifndef _WIN32
        /* set thread stack size - on some systems
         * (e.g. AIX) the default posix thread stack size is very very small */
        cthread_attr_setstacksize( THREAD_STACK_SIZE );
    #endif

    LOG( log_INFO, "Entering main loop" );

    /* Server main loop */
    while( main_loop ) {

        FD_ZERO( &addr_set );
        highfd = -1;
        if( IPv6 ) {
            FD_SET( listenfd6, &addr_set );
            if( listenfd6 > highfd ) highfd = listenfd6;
        }
        if( IPv4 ) {
            FD_SET( listenfd4, &addr_set );
            if( listenfd4 > highfd ) highfd = listenfd4;
        }

        /* wait for an incoming connection */
        if( (select(highfd + 1, &addr_set, NULL, NULL, NULL) < 0 ) ) {
            if( exit_sig_caught ) {
                if( FD_ISSET( listenfd4, &addr_set ) ) closesocket( listenfd4 );
                if( FD_ISSET( listenfd6, &addr_set ) ) closesocket( listenfd6 );
                break;
            }
            LOG( log_ERROR, "socket error: select" );
            main_exit_code = EXIT_FAILURE;
            goto label_exit;
        }

        ++hit;

        /* allocate memory for webserver thread arguments */
        arguments = (thread_arg_t*)malloc( sizeof(thread_arg_t) );
        /* copy baseargs (contains pointers to settings and module data) */
        memcpy( arguments, &baseargs, sizeof(thread_arg_t) );

        if( IPv6 && FD_ISSET( listenfd6, &addr_set ) ) {
            length = sizeof(cli_addr6);
            arguments->fd = (int)accept( listenfd6, (struct sockaddr *)&(cli_addr6), &length );
        }
        else if( IPv4 && FD_ISSET( listenfd4, &addr_set ) ) {
            length = sizeof(cli_addr4);
            arguments->fd = (int)accept( listenfd4, (struct sockaddr *)&(cli_addr4), &length );
        }

        if( arguments->fd < 0 ) {
            free( arguments );

            if( !exit_sig_caught ) {
                LOG( log_ERROR, "accept error (hit %lu)", hit );
                continue;
            }

            if( FD_ISSET( listenfd4, &addr_set ) ) closesocket( listenfd4 );
            if( FD_ISSET( listenfd6, &addr_set ) ) closesocket( listenfd6 );
            break;
        }
        if( IPv6 && FD_ISSET( listenfd6, &addr_set ) ) {
            arguments->client_addr = malloc(INET6_ADDRSTRLEN); arguments->client_addr[0] = 0;
            inet_ntop( AF_INET6, &(cli_addr6.sin6_addr), arguments->client_addr, INET6_ADDRSTRLEN, &cli_addr6);
            arguments->client_port = cli_addr6.sin6_port;
        }
        else if( IPv4 && FD_ISSET( listenfd4, &addr_set ) ) {
            arguments->client_addr = malloc(INET_ADDRSTRLEN); arguments->client_addr[0] = 0;
            inet_ntop( AF_INET, &(cli_addr4.sin_addr), arguments->client_addr, INET_ADDRSTRLEN, &cli_addr4);
            arguments->client_port = cli_addr4.sin_port;
        }
        arguments->hit = hit;

        /* create webthread to handle the incoming request. */
        if( !cthread_create( &mythread, webthread, arguments ) ) {
            LOG( log_ERROR, "thread creation error (hit %lu)", hit );
            if( FD_ISSET( listenfd4, &addr_set ) ) closesocket( listenfd4 );
            if( FD_ISSET( listenfd6, &addr_set ) ) closesocket( listenfd6 );
            free( arguments );
        }
        else
            cthread_detach( &mythread );

    } /* while(main_loop) */


    /* exit label */
    label_exit:
    LOG( log_ALWAYS, "exiting..." );
    label_nolog_exit:

    #ifdef _WIN32
        WSACleanup();
    #endif

    /* clean up */
    server_deinitialize( &baseargs );

    return;
}

static void server_deinitialize( thread_arg_t *args )
{
    if( args == NULL ) return;

    webthread_free( args->pDataSrvThreads );
    server_commands_free( args->pDataSrvCmds );
    websession_free( args->pDataSrvSessions );
    settings_free( (server_settings_t*)args->pSettings );
    #if LUA_SUPPORT
        luasp_free( args->pDataLuaScripting );
    #endif
    /* deinitialize logging at the end, other modules might want to
     * log something in their free function.. */
    log_deinitialize();
}

/* main entry */
int main(int argc, char** argv)
{
    /* call webserver main */
    server_main(argc, argv);
    return main_exit_code;
}
