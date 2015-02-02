/** @file settings.c
 *	@author Jahn Fuchs
 *
 */

#include <string.h>
#include <stdlib.h>

#include "config.h"

#include "settings.h"
#include "ini_reader.h"
#include "cfile.h"
#include "log.h"

#define WEBSRV_PORT_DEFAULT 8181
#define LUASP_SESSION_TIMEOUT_DEFAULT 1800

#define INI_SECTION_SERVER 		    "server"
#define INI_SECTION_SCRIPTING	    "scripting"
#define INI_SECTION_SCRIPTING_CACHE "scripting_cache"

/*
 * read an ini settings file, following settings are supported (# are optional)
 * some settings can be overwritten via command line options..
 *
 *  [server]
 *# wwwroot= www root directory, none by default
 *# port= server port, 8181 default
 *# logfile = logfile, ~/mws.log by default
 *# loglevel_file = 0 for off, 1-6 for log levels (always, error, warning, info, debug, verbose) -  2 by default (warning)
 *# loglevel_console = 0 for off, 1-6 for log levels (always, error, warning, info, debug, verbose) -  1 by default (error)
 *# ipv6 = 1 or 0, 1 by default / enables or disables ipv6 support
 *# deflate = 0-9, 0 is off = default, 1-9 is compression level, while 1 is the fastest and 9 the best compression
 *# disable_embedded_res = 0 or 1, 0 by default
 *# [scripting]
 *# enabled = 0 or 1,  1 by default
 *# error_output = 1 or 0, 1 by default
 *# session_timeout = ..     1800 by default
 *# caching = 0 or 1, 0 by default
 *#
 *# [scripting_cache]  ; only used if scripting.caching = 1
 *# cache_tmpfile = 0 or 1, 1 by default (if scripting caching =1)
 *# cache_memory = 0 or 1, 0 by default
 *# cache_memory_limit_mb = [max cache size in mb], 10 by default
 *# cache_tmpfile_limit_mb = [max cache size in mb], 50 by default
 *#
 *
 */

/* init webserver settings */
server_settings_t * settings_init( void ) {
    server_settings_t *pSettings = malloc( sizeof(server_settings_t) );
    if( pSettings != NULL ) {
        memset( pSettings, 0, sizeof(server_settings_t) );
        pSettings->logfile = NULL;
        pSettings->wwwroot = NULL;

        pSettings->loglevel_file = SETTING_VAL_NOT_SET;
        pSettings->loglevel_console = SETTING_VAL_NOT_SET;

        pSettings->port = WEBSRV_PORT_NOT_SET;
        pSettings->disable_er = SETTING_VAL_NOT_SET;
        #if DEFLATE_SUPPORT
            pSettings->deflate = SETTING_VAL_NOT_SET;
        #endif
        #if LUA_SUPPORT
            pSettings->scripting.enabled = SETTING_VAL_NOT_SET;
            pSettings->scripting.cache = SETTING_VAL_NOT_SET;
            pSettings->scripting.error_output = 1;
        #endif
    }
    return pSettings;
}

/* free webserver settings */
void settings_free(server_settings_t *pSettings ) {
    if( NULL == pSettings ) return;
    free( pSettings->wwwroot );
    free( pSettings->logfile );
    free( pSettings );
}

/* helper function: set logfile to a default value */
static void _settings_SetDefaultLogFile( server_settings_t *pSettings ) {

    const char * homedir = cfile_get_homedir();
    static const char * logfile = "mws.log";

    if( pSettings->logfile )
        free( pSettings->logfile );

    pSettings->logfile = NULL;

    if( homedir && ( pSettings->logfile = malloc( strlen(homedir) + 1 +  strlen(logfile) + 1) ) )
        sprintf( pSettings->logfile, "%s%c%s", homedir, DIR_SEP, logfile);
}

/* helper function: set settings to a default value */
static void _settings_setdefault( server_settings_t *pSettings, int OverwriteExisting ) {

    if( OverwriteExisting || pSettings->loglevel_file == SETTING_VAL_NOT_SET )
        pSettings->loglevel_file = log_WARNING;

    if( OverwriteExisting || pSettings->loglevel_console == SETTING_VAL_NOT_SET )
        pSettings->loglevel_console = log_ERROR;

    pSettings->ipv6 = 1;

    if( OverwriteExisting || pSettings->port == WEBSRV_PORT_NOT_SET )
        pSettings->port = WEBSRV_PORT_DEFAULT;

    #if LUA_SUPPORT
        if( OverwriteExisting || pSettings->scripting.enabled == SETTING_VAL_NOT_SET )
            pSettings->scripting.enabled = 1;
        pSettings->scripting.error_output = 1;
        pSettings->scripting.session_timeout = LUASP_SESSION_TIMEOUT_DEFAULT;
        if( OverwriteExisting || pSettings->scripting.cache == SETTING_VAL_NOT_SET )
            pSettings->scripting.cache = LUASP_CACHING_NONE;
    #endif
    #if DEFLATE_SUPPORT
        if( OverwriteExisting || pSettings->deflate == SETTING_VAL_NOT_SET )
            pSettings->deflate = 0;
    #endif
    if( OverwriteExisting || pSettings->disable_er == SETTING_VAL_NOT_SET )
        pSettings->disable_er = 0;
}

/* load settings from an INI file into webserver settings */
int settings_loadini(server_settings_t *pSettings, const char* filename, int DefaultOnError ) {

    ini_dictionary *ini = NULL;
    char * str;
    char filename_temp[1024];

    /* try to read config file */
    if( ini_readfile( filename, &ini, NULL ) != INI_OKAY ) {
        // nothing here, ini is set to NULL by ini_readfile
    }

    /* if failed, try read config file in users home directory */
    if( !ini && DefaultOnError ) {
        const char *hdir = cfile_get_homedir();
        if( hdir && strlen(hdir)+1+strlen(filename) < sizeof(filename_temp) ) {
            sprintf(filename_temp, "%s%c%s", cfile_get_homedir(), DIR_SEP, filename);
            if( ini_readfile( filename_temp, &ini, NULL ) != INI_OKAY ) {
                /* nothing here, ini is set to NULL by ini_readfile */
            }
        }
    }

    /* on non-windows systems try to read config file in /etc */
    #ifndef _WIN32
        if( !ini && DefaultOnError ) {
            if( strlen("/etc/")+strlen(filename) < sizeof(filename_temp) ) {
                sprintf(filename_temp, "/etc/%s", filename);
                if( ini_readfile( filename, &ini, NULL ) != INI_OKAY ) {
                    /* nothing here, ini is set to NULL by ini_readfile */
                }
            }
        }
    #endif

    if( !ini ) {
        if( DefaultOnError ) {
            _settings_setdefault( pSettings, 0 );
            return 1;
        }
        return 0;
    }

    /* get HomeDir */
    if( !pSettings->wwwroot ) {
        str = ini_dictionary_getstring(ini, INI_SECTION_SERVER, "wwwroot", NULL);
        if( str && str[0] ) {
            pSettings->wwwroot = malloc( strlen(str) + 2 );
            strcpy(pSettings->wwwroot, str);
            /* make sure wwwroot has a trailing directory seperator character */
            if( pSettings->wwwroot[strlen(str)-1] != DIR_SEP ) {
                pSettings->wwwroot[strlen(str)+1] = '\0';
                pSettings->wwwroot[strlen(str)] = DIR_SEP;

            }
        }
    }

    /* get LogFile */
    if( !pSettings->logfile ) {
        str = ini_dictionary_getstring(ini, INI_SECTION_SERVER, "logfile", NULL);
        if( str && str[0] ) {
            if( (pSettings->logfile = malloc( strlen(str) + 1 )) )
                strcpy( pSettings->logfile, str );
        } else {
            _settings_SetDefaultLogFile( pSettings );
        }
    }

    if( pSettings->port == WEBSRV_PORT_NOT_SET )
        pSettings->port = ini_dictionary_getint( ini, INI_SECTION_SERVER, "port", WEBSRV_PORT_DEFAULT );

    pSettings->loglevel_file = ini_dictionary_getint( ini, INI_SECTION_SERVER, "loglevel_file", log_WARNING );
    pSettings->loglevel_console = ini_dictionary_getint( ini, INI_SECTION_SERVER, "loglevel_console", log_ERROR );
    pSettings->ipv6 = ini_dictionary_getboolean( ini, INI_SECTION_SERVER, "ipv6", 1 );

    if( pSettings->disable_er == SETTING_VAL_NOT_SET )
        pSettings->disable_er = ini_dictionary_getint( ini, INI_SECTION_SERVER, "disable_embedded_res", 0 );


    #if DEFLATE_SUPPORT
    if( pSettings->deflate == SETTING_VAL_NOT_SET )
        pSettings->deflate = ini_dictionary_getint( ini, INI_SECTION_SERVER, "deflate", 0 );
        if( pSettings->deflate > 9 ) pSettings->deflate = 9;
        else if( pSettings->deflate < 0 ) pSettings->deflate = 0;
    #endif

    /* load scripting settings --------------------------------------------------- */
    #if LUA_SUPPORT
        if( pSettings->scripting.enabled == SETTING_VAL_NOT_SET )
                pSettings->scripting.enabled = ini_dictionary_getboolean( ini, INI_SECTION_SCRIPTING, "enabled", 1 );
        pSettings->scripting.error_output = ini_dictionary_getboolean( ini, INI_SECTION_SCRIPTING, "error_output", 1 );
        pSettings->scripting.session_timeout = ini_dictionary_getint( ini, INI_SECTION_SCRIPTING, "session_timeout", LUASP_SESSION_TIMEOUT_DEFAULT );
        if( pSettings->scripting.cache == SETTING_VAL_NOT_SET ) {
            pSettings->scripting.cache = LUASP_CACHING_NONE;
            if( ini_dictionary_getboolean( ini, INI_SECTION_SCRIPTING, "caching", 0 ) ) {
                if( ini_dictionary_getboolean( ini, INI_SECTION_SCRIPTING_CACHE, "cache_tmpfile", 1 ) ) {
                    pSettings->scripting.cache |= LUASP_CACHING_FILE;
                }
                if( ini_dictionary_getboolean( ini, INI_SECTION_SCRIPTING_CACHE, "cache_memory", 0 ) ) {
                    pSettings->scripting.cache |= LUASP_CACHING_MEMORY;
                }
            }
        }
    #endif

    ini_dictionary_free( ini );
    return 1;
}
