/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#include "cmdline.h"
#include "settings.h"
#include "log.h"
#include "version.h"
#include "cfile.h"

#if LUA_SUPPORT
#include <lua.h>
#endif

/* Print help to console */
void cmdline_print_help( const char *name, const int detail )
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
			"\n\n"
			"usage: %s <options>\n\n"
			"   options:\n"
			"     -h, -?          display full help and exit \n"
			"     -p  {port}      set port via command line\n"
			"     -d  {dir}       set www root dir\n"
			"     -c  {file}      load specific config file\n"
			"     -l  {file}      specify logfile output\n",
			name);
	if( detail > 0 ) {
	printf( "     -d  {dir}       set www root dir\n"
			"     -rd             disable embedded resources\n"
			"     -llf            log level to file\n"
			"     -llc            log level to console\n"
			"                     valid log levels are 0-6\n"
			"                     0 - disabled, 1 - always, 2 - error, 3 - warning\n"
			"                                   4 - info,   5 - debug, 6 - verbose\n");
	}
#if DEFLATE_SUPPORT
	if( detail > 0 )
	printf( "     -deflate {0-9}  turn compression support off (0) or on (1-9)\n"
			"                     default value is off (0)\n");
#endif
#if LUA_SUPPORT
	if( detail > 0 ) {
        printf( "     -luasp {0,1}    turn lua scripting on (1) or off (0)\n"
                "                     default value is on\n" );
        printf( "     -sc {[0|mf]}    scripting cache, 0=off, m=memory, f=tmpfile\n"
                "                     options can be mixed, i.e. '-sc mf' or just '-sc f'\n"
                "                     default is off\n" );
	}
#endif
#ifdef _WIN32
#ifdef USE_WINDOWS_SERVICE
	if( detail > 0 )
	printf( "\n"
			"     -svcinstall     register as windows service\n"
			"     -svcuninstall   unregister windows service\n");
#endif
#endif
	printf( "\n"
			"   Example: %s -p 8181 &\n\n", name);
}

int cmdline_parse( thread_arg_t *args, const int argc, char **argv, char ** config_file )
{
	server_settings_t *pSettings = args->pSettings;
	int i=1, err=CMDLINE_OKAY;

    /* check command line arguments */
	for( ; i < argc; ++i ) {

		if( !strcmp(argv[i], "-p") ) {
			if( i+1 < argc && ++i ) {
				pSettings->port = atoi( argv[i] );	/* set port */
			}
			else {
				fprintf(stderr, "%s option needs argument\n\n", argv[i]);
				err = CMDLINE_FORMAT_ERROR;
			}
		}
		else if( !strcmp(argv[i], "-c") ) {
			if( i+1 < argc && ++i ) {
				*config_file = argv[i]; /* set config file to load */
			}
			else {
				fprintf(stderr, "%s option needs argument\n\n", argv[i]);
				err = CMDLINE_FORMAT_ERROR;
			}
		}
		else if( !strcmp(argv[i], "-d") ) {
			if( i+1 < argc && ++i ) {
				/* set www root dir */
				if( pSettings->wwwroot != NULL )
					free( pSettings->wwwroot );
				if( (pSettings->wwwroot  = malloc( strlen(argv[i]) + 2 )) ) {
					strcpy( pSettings->wwwroot, argv[i] );
					if( pSettings->wwwroot[strlen(argv[i])-1] != DIR_SEP) {
						pSettings->wwwroot[strlen(argv[i])+1] = '\0';
						pSettings->wwwroot[strlen(argv[i])] = DIR_SEP;
					}
				}
			}
			else {
				fprintf(stderr, "%s option needs argument\n\n", argv[i]);
				err = CMDLINE_FORMAT_ERROR;
			}
		}
		else if( !strcmp(argv[i], "-rd") ) {
			/* Disable embedded resource lookups */
			pSettings->disable_er = 1;
		}
#if DEFLATE_SUPPORT 
		else if( !strcmp(argv[i], "-deflate") ) {
			if( i+1 < argc && ++i ) {
				/* set deflate option */
				pSettings->deflate = atoi( argv[i] );
				if( pSettings->deflate < 0 || pSettings->deflate > 9 || !strlen(argv[i]) ) {
					fprintf(stderr, "%s option needs argument between 0-9\n\n", argv[i-1]);
					err = CMDLINE_FORMAT_ERROR;
				}
			}
			else {
				fprintf(stderr, "%s option needs argument\n\n", argv[i]);
				err = CMDLINE_FORMAT_ERROR;
			}
		}
#endif
#if LUA_SUPPORT
		else if( !strcmp(argv[i], "-luasp") ) {
			if( i+1 < argc && ++i ) {
				/* set scripting enable option */
				pSettings->scripting.enabled = atoi( argv[i] );
				if( pSettings->scripting.enabled < 0
				        || pSettings->scripting.enabled > 1
				        || !strlen(argv[i]) ) {
					fprintf(stderr, "%s option needs argument 0 or 1\n\n", argv[i-1]);
					err = CMDLINE_FORMAT_ERROR;
				}
			}
			else {
				fprintf(stderr, "%s option needs argument\n\n", argv[i]);
				err = CMDLINE_FORMAT_ERROR;
			}
		}
        else if( !strcmp(argv[i], "-sc") ) {
            if( i+1 < argc && ++i ) {
                /* set scripting cache option */
                size_t len = strlen( argv[i] );
                size_t j = 0;
                pSettings->scripting.cache = 0;
                for( ; j < len; ++j ) {
                    if( argv[i][j] == 'm' )
                        pSettings->scripting.cache |= LUASP_CACHING_MEMORY;
                    else if( argv[i][j] == 'f' )
                        pSettings->scripting.cache |= LUASP_CACHING_FILE;
                    else if( argv[i][j] == '0' )
                        pSettings->scripting.cache |= LUASP_CACHING_NONE;
                    else {
                        fprintf(stderr, "%s option has unknown argument '%c'\n\n", argv[i-1], argv[i][j]);
                        err = CMDLINE_FORMAT_ERROR;
                        break;
                    }
                }
            }
            else {
                fprintf(stderr, "%s option needs argument\n\n", argv[i]);
                err = CMDLINE_FORMAT_ERROR;
            }
        }
#endif
		else if( !strcmp(argv[i], "-l") ) {
			if( i+1 < argc && ++i ) {
				/* set log file */
				free ( pSettings->logfile );
				if( (pSettings->logfile = malloc( strlen(argv[i]) + 1 )) )
					strcpy( pSettings->logfile, argv[i] );
			}
			else {
				fprintf(stderr, "%s option needs argument\n\n", argv[i]);
				err = CMDLINE_FORMAT_ERROR;
			}
		}
		else if( !strcmp(argv[i], "-llf") ) {
			if( i+1 < argc && ++i ) {
				/* set file log level */
				int level = atoi( argv[i] );
				if( level >= 0 && level <= log_VERBOSE )
    				pSettings->loglevel_file = level;
    			else {
    				fprintf(stderr, "%s option has invalid value %d\n\n", argv[i-1], level);   
				    err = CMDLINE_FORMAT_ERROR;
				}
			}
			else {
				fprintf(stderr, "%s option needs argument\n\n", argv[i]);
				err = CMDLINE_FORMAT_ERROR;
			}
		}
		else if( !strcmp(argv[i], "-llc") ) {
			if( i+1 < argc && ++i ) {
				/* set console log level */
				int level = atoi( argv[i] );
				if( level >= 0 && level <= log_VERBOSE )
    				pSettings->loglevel_console = level;
    			else {
    				fprintf(stderr, "%s option has invalid value %d\n\n", argv[i-1], level);   
				    err = CMDLINE_FORMAT_ERROR;
				}
			}
			else {
				fprintf(stderr, "%s option needs argument\n\n", argv[i]);
				err = CMDLINE_FORMAT_ERROR;
			}
		}
		#ifdef _WIN32
		else if(!strcmp(argv[i], "-service")) {
			/* skip -service option */
		}
		#endif
		else {
			if( strcmp(argv[i], "-h") && strcmp(argv[i], "-H") && strcmp(argv[i], "-?") ) {
				/* unknown arguments */
				fprintf(stderr, "unknown command line option '%s'\n\n", argv[i]);
				err = CMDLINE_FORMAT_ERROR;
			}
			else return CMDLINE_HELP_REQUESTED;
		}
	}
	return err;
}
