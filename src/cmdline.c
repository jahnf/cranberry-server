/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmdline.h"

#include "config.h"
#include "version.h"
#include "log.h"

#include "settings.h"
#include "ip_socket_utils.h"
#include "cfile.h"
#include "optparse.h"

#if LUA_SUPPORT
#include <lua.h>
#endif

void cmdline_print_version( const char *name )
{
	printf( "%s (%s", name, get_version_string());

	switch( get_build_type() ) {
	case BUILD_TYPE_DEBUG:
	    printf(", debug)"); break;
	case BUILD_TYPE_RELEASE:
	    printf(", release)"); break;
	default:
	    printf(", unknown build)"); break;
	}

	printf( "\n"
    #if DEFLATE_SUPPORT || LUA_SUPPORT
			" [compiled with: "
    #endif
    #if DEFLATE_SUPPORT
			"deflate"
    #endif
    #if DEFLATE_SUPPORT && (LUA_SUPPORT || SQLITE_SUPPORT)
			", "
    #endif
    #if LUA_SUPPORT
			LUA_VERSION
    #endif
    #if DEFLATE_SUPPORT && LUA_SUPPORT && SQLITE_SUPPORT
            ", "
    #endif
    #if SQLITE_SUPPORT
            "sqlite3"
    #endif
    #if DEFLATE_SUPPORT || LUA_SUPPORT || SQLITE_SUPPORT
			"]"
    #endif
            "\n");
}

static void cmdline_print_option_list(struct optparse_long *longopts)
{
    struct optparse_long *lo;
    int longname_maxlen = 0;
    for( lo = longopts; lo != NULL && 
         !(lo->shortname == 0 && lo->longname == NULL); ++lo ) {
        if( lo->help_text == NULL ) continue;
        if( lo->longname && longname_maxlen < strlen(lo->longname) )
            longname_maxlen = strlen(lo->longname);
    }
    
    for( lo = longopts; lo != NULL && 
         !(lo->shortname == 0 && lo->longname == NULL); ++lo ) {
        if( lo->help_text == NULL ) continue;

        if( lo->shortname != 0 )
            printf(" -%c", lo->shortname);
        else 
            printf("    ");

        if( lo->shortname != 0 && lo->longname )
            printf(", ");
        else
            printf("  ");

        if( lo->longname ) {
            int length_rest = longname_maxlen - strlen(lo->longname) + 2;
            printf("--%s", lo->longname);
            for( ; length_rest; -- length_rest ) printf(" ");
        } else {
            int length_rest = longname_maxlen + 4;
            for( ; length_rest; -- length_rest ) printf(" ");
        }

        printf("%s\n", lo->help_text);
    }
}

static struct optparse_long cmd_longopts[] = {
    {"version",          'v', OPTPARSE_NONE, "Print version"},
    {"help",             'h', OPTPARSE_NONE, "Print help"},
    {"port",             'p', OPTPARSE_REQUIRED, "Set server port"},
    {"config",           'c', OPTPARSE_REQUIRED, "Set config file"},
    {"webroot",          'r', OPTPARSE_REQUIRED, "Set www root directory"},
    {"logfile",          'l', OPTPARSE_REQUIRED, "Set log file"},
    {"loglevel-file",    'F', OPTPARSE_REQUIRED, "Set loglevel for logfile"},
    {"loglevel-console", 'C', OPTPARSE_REQUIRED, "Set loglevel for console"},
    #if DEFLATE_SUPPORT
    {"deflate",          'd', OPTPARSE_REQUIRED, "Set deflate compression level"},
    #endif
    {NULL,               'D', OPTPARSE_NONE, "Disable embedded resources"},
    {0}
};

/* Print help to console */
void cmdline_print_help( const char *name, const int detail )
{
    printf("Usage: %s [OPTIONS]\n\n", name);
    cmdline_print_option_list( cmd_longopts );
	printf( "\n  Example: %s -p 8181 &\n\n", name);
}

int cmdline_parse( const char * name, thread_arg_t *args, const int argc, 
                   char **argv, char ** config_file )
{
	server_settings_t *pSettings = args->pSettings;
	int err=CMDLINE_OKAY; 

    struct optparse options;
    int option;
    optparse_init(&options, argv);

    while ((option = optparse_long(&options, cmd_longopts, NULL)) != -1) {
        switch (option) {
        case 'v':
            return CMDLINE_VERSION_REQUESTED;
            break;
        case 'h':
            return CMDLINE_HELP_REQUESTED;
            break;
        case 'p':
            pSettings->port = atoi( options.optarg );
            if( pSettings->port < SERVER_PORT_MIN || pSettings->port > SERVER_PORT_MAX ) {
                fprintf(stderr, "'%c' option needs to be between %d-%d\n", 
                        option, SERVER_PORT_MIN, SERVER_PORT_MAX);
                err = CMDLINE_FORMAT_ERROR;
            }
            break;
        case 'c':
            *config_file = (char*)options.optarg;
            break;
        case 'r':
            /* set www root dir */
            if( pSettings->wwwroot != NULL )
                free( pSettings->wwwroot );
            if( (pSettings->wwwroot  = malloc( strlen(options.optarg) + 2 )) ) {
                strcpy( pSettings->wwwroot, options.optarg );
                /* Make sure directory ends with a trailing directory separator */
                if( pSettings->wwwroot[strlen(options.optarg)-1] != DIR_SEP) {
                    pSettings->wwwroot[strlen(options.optarg)+1] = '\0';
                    pSettings->wwwroot[strlen(options.optarg)] = DIR_SEP;
                }
            }
            break;
        case 'D':
            pSettings->disable_er = 1;
            break;
        case 'l':
            free( pSettings->logfile );
            if( (pSettings->logfile = malloc( strlen(options.optarg) + 1 )) )
                strcpy( pSettings->logfile, options.optarg );
            break;
        case 'F': 
            /* set file log level */
            pSettings->loglevel_file = atoi( options.optarg );
            if( pSettings->loglevel_file < log_DISABLED 
                || pSettings->loglevel_file > log_VERBOSE ) {
                fprintf(stderr, "'%c' option has an invalid log level value.\n"
                                "Log levels range from %d (disabled) to %d (verbose)\n", 
                                option, log_DISABLED, log_VERBOSE);   
                return CMDLINE_FORMAT_ERROR;
            }
            break;
        case 'C': 
            /* set console log level */
            pSettings->loglevel_console = atoi( options.optarg );
            if( pSettings->loglevel_console < log_DISABLED 
                || pSettings->loglevel_console > log_VERBOSE ) {
                fprintf(stderr, "'%c' option has an invalid log level value.\n"
                                "Log levels range from %d (disabled) to %d (verbose)\n", 
                                option, log_DISABLED, log_VERBOSE);   
                return CMDLINE_FORMAT_ERROR;
            }
            break;
        #if DEFLATE_SUPPORT 
        case 'd':
            pSettings->deflate = atoi( options.optarg );
            if( pSettings->deflate < 0 || pSettings->deflate > 9 || !strlen(options.optarg) ) {
                fprintf(stderr, "'%c' option needs to be between 0-9\n", option);
                err = CMDLINE_FORMAT_ERROR;
            }
            break;
        #endif
        case '?':
            fprintf(stderr, "%s: %s\n", name, options.errmsg);
            err = CMDLINE_FORMAT_ERROR;
        }
    }
	return err;
}    
