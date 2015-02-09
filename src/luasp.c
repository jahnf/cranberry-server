/** @file luasp.c
 * Implementation of embedded lua scripting.
 * The webserver can use embedded scripting inside dynamic pages (Lua server pages).
 * For a complete description of the Lua scripting language go to http://www.lua.org
 * Currently Lua 5.3 is compiled and included with the server.
 *
 * Inside Lua server pages (*.lsp), script tags known from PHP can be used:
 * <? ...  some Lua code ... ?>, also <?=some_lua_var?> can be used.
 * 
 * The Lua environment is set up and the following additional functions exist:
 * - echo( ... ), write ( ... )
 *    Variable number of arguments, print out arguments to current http_buffer.
 *    If no http headers were sent before the first echo/write call,
 *    the http headers are sent automatically.
 * - http_header( header_field [, value] )
 *    Set a http header field to a certain value. This function must be called
 *    before any echo/write call or html output to have any effect. If the second parameter 
 *    is 'nil' the header field is unset if existing. If called with the header_field
 *    argument only, the value of that header field is returned 
 *    (or nil if the header field is not set).
 *    CAUTION: Common header fields like 'Date', 'Transfer-Encoding' and 
               others might get overwritten by the web server.
 * - http_response_code( [code] )
 *    Sets and respectably gets the http return code which is about to be sent,
 *    must be called before any echo/write call or html output to have any effect.
 * - session_start( [max_age_in_sec] )
 *    Creates a session or resumes the current one based on a session identifier
 *    passed via a cookie. The env.session table is set accordingly.
 *    Returns the session id as a string. In most cases it's necessary to call
 *    session_start() before the first echo/write/html output, so that the 'Set-Cookie'
 *    header is set and sent correctly. max_age_in_sec defaults to LUASP_SESSION_TTL_DEFAULT
 *    which is 1800 seconds (30min) by default.  If max_age_in_sec is a negative
 *    number an expire date in the past is sent to the browser (Which will tell
 *    browser that the cookie is expired)
 * - session_var( var_name [, var_value] )
 *    Set or get session variables. The global env.session table is also set accordingly.
 *    To unset a session variable set var_value to nil. This function needs
 *    a prior call to session_start() to have any effect.
 * - session_destroy()
 *    Destroys a session and the associated session variables.
 *    Call this function after a call to session_start() to have any effect.
 *    Call this function before any echo/write/html output, to have any effect on
 *    the set-cookie header field (sets the cookie expire-field to the past which
 *    will tell the browser to set the cookie to expired)
 *
 *  ------------ Lua Variables
 *  Different environment variables are set up and available in the Lua script parts.
 *  Server settings, get, post and cookies can be accessed from scripts.
 *
 *  env.server.remote_addr
 *  env.server.remote_port
 *  env.server.server_port
 *  env.server.www_root
 *  env.server.script
 *  env.server.request_method
 *  env.server.server_version
 *
 *  env.get_vars
 *  env.post_vars
 *  env.cookies
 *  env.headers
 *  env.session
 *
 *
 */

#include "luasp.h"
#if LUA_SUPPORT

#include "luasp_types.h"
#include "luasp_common.h"
#include "luasp_reader.h"
#include "luasp_session.h"
#include "luasp_cache.h"

#include "http_defines.h"
#include "websession.h"
#include "str_utils.h"
#include "settings.h"
#include "cfile.h"
#include "log.h"

#include "cresource.h"
#include "version.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>

SETLOGMODULENAME("luasp");

#ifndef __cplusplus
    #ifdef _MSC_VER
    /* inline keyword is not available in Microsoft C Compiler */
    #define inline __inline
    #endif
#endif

/* Uncomment the following if you want to keep \r & \n after ?> closing tags */
/*#define LUASP_LEAVE_LF_UNTOUCHED*/

/** luasp initialization data */
typedef struct {
    void *cache; /**< needs to be set for caching */
} luasp_idata_t;

/* init and free functions. later used to init cache and other things.. */
void * luasp_init( thread_arg_t *args ) {
    //const WebSrvSettings *pSettings = args->pSettings;
    luasp_idata_t *data = malloc(sizeof(luasp_idata_t));

    if( data != NULL ) {
        data->cache = NULL;
//        if( pSettings->scripting.cache ) {
//
//            data->cache = luasp_cache_init( args );
//
////            const char *tempdir;
////            if( (tempdir = cfile_get_tempdir()) ) {
////
////                if( (data->tempdir = malloc(strlen(tempdir)+1)) ) {
////                    strcpy( data->tempdir, tempdir );
////                    LOG_PRINT( args, log_INFO, "%s", data->tempdir );
////                }
////            }
//
////            if( data->tempdir ) {
////                /* TODO init caching here */
////                /*
////                 * - luasp cache:
////                 *   - file caching, cache lua binary chunks to temp files
////                 *          - setting of max cache size, items
////                 *
////                 *   - LATER: memory caching, cache lua binary chunks to memory:
////                 *         need settings for max cache size, max cache items etc...
////                 *         setting: oldest cached junks go to disk if file caching on and
////                 *                  over max cache size, items...
////                 *
////                 *         -> need luasp_reader for memory cached files
////                 *             ( part of chunk -> part of chunk ->...)
////                 *
////                 *   - functions to find cached items
////                 *   - functions to remove item from cache
////                 *   - clear cache function
////                 *   - init cache function
////                 *
////                 */
////            }
//        }
    }
    return data;
}

void luasp_free( void *data_in ) {
    luasp_idata_t *data = data_in;
    if( data == NULL ) return;

    luasp_cache_free( data->cache );
    free( data );
}

/* send http headers, if the content-type field ist not set
 * the function uses the default text/html content type */
static void _lsp_send_headers( luasp_page_state_t *es ) {
    if( !es->headers )
        send_buffer_simple_http_header( es->args->sendbuf, es->http_status_code,
                                        HTTP_CONTENT_TYPE_HTML, es->ri->http_version );
    else {
        kv_item *content_type = kvlist_find_key( HTTP_HEADER_CONTENT_TYPE, es->headers );
        /* if no content-type header was set use default html content type */
        if( !content_type )
            es->headers = kvlist_new_item_push_front( HTTP_HEADER_CONTENT_TYPE, HTTP_CONTENT_TYPE_HTML, es->headers );
        send_buffer_http_header( es->args->sendbuf, es->http_status_code,
                                 es->headers, es->ri->http_version );
    }
    es->headers_sent = 1;
}


/* C implementation of the luasp echo/write function */
static int lsp_echo( lua_State *L ) 
{
    luasp_page_state_t *es;
    const int n = lua_gettop(L);
    int i;

    lua_getglobal( L, LUASP_GLOB_USERDATA_NAME );
    es = lua_touserdata( L, -1 );

    /* check if http headers have been sent already..
     * (we need to do so before first output) */
    if( n && !es->headers_sent )
        _lsp_send_headers( es );

    {
        size_t len;
        const char* s;
        for( i=1; i<=n; ++i ) {
            len = 0;
            s = lua_tolstring( L, i, &len );
            send_buffer_string_data( es->args->sendbuf, (char*)s, len );
        }
    }
    return 0;
}

/* C implementation of the luasp http_response code function */
static int lsp_http_response_code( lua_State *L ) 
{
    luasp_page_state_t *es;
    const int n = lua_gettop(L);

    lua_getglobal( L, LUASP_GLOB_USERDATA_NAME );
    es = lua_touserdata( L, -1 );

    if( n )  /* if at least 1 argument */
        es->http_status_code = (int)lua_tointeger( L, 1 );

    lua_pushinteger( L, es->http_status_code );
    return 1;
}

/* C implementation of the luasp http_header function */
static int lsp_http_header( lua_State *L ) 
{
    luasp_page_state_t *es;
    const int n = lua_gettop(L);

    if( n < 1 || n > 2 )
        luaL_error( L, "incorrect number of arguments" );

    lua_getglobal( L, LUASP_GLOB_USERDATA_NAME );
    es = lua_touserdata( L, -1 );

    if( n == 1 ) {
        const char *field = lua_tostring( L, 1 );
        kv_item *exists = kvlist_find_key( field, es->headers );
        if( exists && exists->value )
            lua_pushstring( L, exists->value );
        else
            lua_pushnil( L );
        return 1;
    } else if( n == 2 ){
        kv_item *exists;
        const char *field = lua_tostring( L, 1 );

        exists = kvlist_find_key( field, es->headers );

        /* if second parameter is nil - remove http header from header list*/
        if( lua_isnil(L,2) ) {
            if( exists ) {
                kvlist_remove_item( &es->headers, exists );
                exists = NULL;
            }
        }
        else { /* second parameter is not nil - add http header to list or update existing one*/
            size_t vlen;
            const char *value = lua_tolstring( L, 2, &vlen );
            if( exists ) {
                if( exists->value != NULL ) free( exists->value );
                exists->value = malloc( vlen+1 );
                strcpy( exists->value, value );
            } else {
                es->headers = kvlist_new_item_push_front( field, value, es->headers );
            }
        }
    }
    return 0;
}

/* Helper function to set up global lua tables and variables for 
 * the Lua state with informations from the HTTP request 
 * and with general web server settings. */
inline
static void _fill_environment( lua_State *L, const http_req_info_t *ri, const thread_arg_t *args )
{
    const server_settings_t *pSettings = args->pSettings;
    kv_item *iter;

    lua_newtable( L );
    /* server: (like the $_SERVER variable in php ) */
    lua_pushstring( L, "server");
    lua_newtable( L );
    lua_set_tablefield_string ( L, "remote_addr", args->client_addr );
    lua_set_tablefield_integer( L, "remote_port", args->client_port );
    lua_set_tablefield_integer( L, "server_port", pSettings->port );
    lua_set_tablefield_string ( L, "www_root", (pSettings->wwwroot?pSettings->wwwroot:"") );
    lua_set_tablefield_string ( L, "script", ri->filename );
    lua_set_tablefield_string ( L, "request_method", http_request_type_to_str(ri->req_method) );
    lua_set_tablefield_string ( L, "server_version", get_version_string() );
#if DEFLATE_SUPPORT
    lua_set_tablefield_integer( L, "deflate_setting", pSettings->deflate );
#endif
    lua_set_tablefield_boolean( L, "embedded_resources_enabled", !pSettings->disable_er );
    lua_settable(L, -3);

    /* http headers */
    lua_pushstring( L, "headers");
    lua_newtable( L );

    for( iter = ri->header_info; iter; iter = iter->next )
        lua_set_tablefield_string( L, iter->key, iter->value );

    lua_settable(L, -3);
    /* cookies */
    lua_pushstring( L, "cookies");
    lua_newtable( L );

    for( iter = ri->cookie_info; iter; iter = iter->next )
        lua_set_tablefield_string( L, iter->key, iter->value );

    lua_settable(L, -3);
    /* GET vars */
    lua_pushstring( L, "get_vars");
    lua_newtable( L );

    for( iter = ri->get_vars; iter; iter = iter->next ) {
        lua_set_tablefield_string( L, iter->key, iter->value );
    }

    lua_settable(L, -3);
    /* POST vars */
    lua_pushstring( L, "post_vars");
    lua_newtable( L );

    for( iter = ri->post_vars; iter; iter = iter->next )
        lua_set_tablefield_string( L, iter->key, iter->value );

    lua_settable(L, -3);
    /* Empty session table */
    lua_pushstring( L, "session");
    lua_newtable( L );
    lua_settable(L, -3);

    lua_setglobal( L, LUASP_ENV_VAR_NAME );
}

/* Register Lua functions in a given Lua state */
inline
static void _luasp_regfuncs( lua_State *L ) {
    lua_register( L, "echo", lsp_echo );
    lua_register( L, "write", lsp_echo );
    lua_register( L, "http_response_code", lsp_http_response_code );
    lua_register( L, "http_header", lsp_http_header );
    lua_register( L, "session_start", lsp_session_start );
    lua_register( L, "session_destroy", lsp_session_destroy );
    lua_register( L, "session_var", lsp_session_var );
}

#if SQLITE_SUPPORT
#define LUA_SQLITELIBNAME	"sqlite3"
LUAMOD_API int (luaopen_lsqlite3) (lua_State *L);
#endif

/* list of lua libs to load */
static const luaL_Reg lualibs[] = {
    {"_G", luaopen_base},
    {LUA_LOADLIBNAME, luaopen_package},
    {LUA_TABLIBNAME, luaopen_table},
    {LUA_STRLIBNAME, luaopen_string},
    {NULL, NULL}
};

static const luaL_Reg preloadedlibs[] = {
    {LUA_COLIBNAME, luaopen_coroutine},
    {LUA_IOLIBNAME, luaopen_io},
    {LUA_OSLIBNAME, luaopen_os},
    {LUA_BITLIBNAME, luaopen_bit32},
    {LUA_MATHLIBNAME, luaopen_math},
    {LUA_DBLIBNAME, luaopen_debug},
    #if SQLITE_SUPPORT
    {LUA_SQLITELIBNAME, luaopen_lsqlite3},
    #endif
    {NULL, NULL}
};

/* Load all libs in lualibs[] in a given lua state */
inline static void _luasp_openlibs( lua_State *L ) 
{
    const luaL_Reg *lib;
    /* Call open functions from 'loadedlibs' and set results to global table */
    for ( lib = lualibs; lib->func; ++lib ) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);  /* remove lib */
    }
    /* Add open functions from 'preloadedlibs' into 'package.preload' table */
    luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
    for (lib = preloadedlibs; lib->func; ++lib ) {
      lua_pushcfunction(L, lib->func);
      lua_setfield(L, -2, lib->name);
    }
    lua_pop(L, 1);  /* remove _PRELOAD table */
}

inline
static kv_item * _push_cache_control_headers_front( kv_item *headers ) 
{
    static const char cc_value[] = "no-store, no-cache, must-revalidate, "
                                   "post-check=0, pre-check=0";

    /* init default cache limiter headers -------------------- */
    headers = kvlist_new_item_push_front_ll( HTTP_HEADER_CACHE_CONTROL, 13,
                        cc_value, sizeof(cc_value)-1, headers );
    headers = kvlist_new_item_push_front_ll( HTTP_HEADER_EXPIRES, 7,
                        LUASP_DATE_IN_PAST, 29, headers );
    headers = kvlist_new_item_push_front_ll( HTTP_HEADER_PRAGMA, 6,
                        "no-cache", 8, headers );
    return headers;
}

int luasp_process( http_req_info_t *ri, thread_arg_t *args ) 
{
    server_settings_t* pSettings = args->pSettings;
    luasp_state_t lst = {0,0,0,0,LST_INIT,1,0};

    /* check for resources if not disabled */
    if( !pSettings->disable_er ) {
        /* look for file name in embedded resources */
        cresource_t *efile = get_cresource( ri->filename );
        if( efile != NULL ) {
            lst.dp = lst.dp_cur = efile->data;
            lst.dp_end = efile->data + (efile->size);
        }
    }

    /* If no embedded resource was set or found and a
     * www root directory was set, try to find and open the given file. */
    if( !lst.dp && pSettings->wwwroot ) {
        cfile_stat_t st;
        if( cfile_getstat( ri->filename, &st ) == CFILE_SUCCESS ) {
            if( st.type != CFILE_TYPE_REGULAR || !(lst.fp = fopen(ri->filename, "rb")) ) {
                /* not a regular file or error opening file */
                send_buffer_error_info( args->sendbuf, ri->filename, 
                                        HTTP_STATUS_FORBIDDEN, ri->http_version );
                return 0;
            }
        }
    }

    /* If data pointer or file pointer is set */
    if( lst.dp || lst.fp ) {
        lua_State *L;
        luasp_page_state_t es = { args, ri, 0, HTTP_STATUS_OK, NULL, NULL, NULL };
        int status;

        const char* (*luasp_reader_func)( lua_State* L , void *ud, size_t* size )
                = lst.dp?luasp_reader_res:luasp_reader_file;

        /* If HTTP version is 1.1 we send the result in chunked transfer encoding, because we
           don't know the size of the document beforehand */
        if( ri->http_version == HTTP_VERSION_1_1 ) args->sendbuf->flags |= SBF_CHUNKED;

        /* Create a new Lua state */
        if(	(L = luaL_newstate()) == NULL ) {
            /* memory allocation error */
            send_buffer_error_info( args->sendbuf, ri->filename, 
                                    HTTP_STATUS_INTERNAL_SERVER_ERROR, ri->http_version );
            return 1;
        }

        /* call lua load function */
        #if LUA_VERSION_NUM	>= 502
            status = lua_load( L, luasp_reader_func, &lst, ri->filename, NULL );
        #else
            status = lua_load( L, luasp_reader_func, &lst, ri->filename );
        #endif

        if( lst.fp ) {
            fclose( lst.fp );
            lst.fp = NULL;
        }

        es.headers = _push_cache_control_headers_front( es.headers );

        if( status ){
            const char *errmsg = lua_tostring(L,-1);
            if( !es.headers_sent ) {
                _lsp_send_headers( &es );
            }
            if( pSettings->scripting.error_output_socket ) {
                send_buffer_string( args->sendbuf, "Luasp load error: " );
                send_buffer_string( args->sendbuf, errmsg );
            }
            LOG_FILE( log_ERROR, "load: %s:%i, %s", ri->filename, lst.line, errmsg );

        /* if successfully loaded, execute script... */
        } else {
            /* TODO here we could dump the lua chunk to the cache 
               if not already loaded from a cache */
            /* TODO else we need to register functions and load libs*/
            
            /* Register Luasp functions */
            _luasp_regfuncs( L );
            
            /* Init and register global user data variable: */
            /* register global user data variable */
            lua_pushlightuserdata( L, &es );
            lua_setglobal( L, LUASP_GLOB_USERDATA_NAME );
            /* open lua's default libraries: */
            _luasp_openlibs( L );

            /* register luasp environment --------------------------------- */
            _fill_environment( L, ri, args );

            if( (status = lua_pcall(L, 0, LUA_MULTRET, 0)) ) {
                const char *errmsg = lua_tostring(L,-1);
                if( !es.headers_sent ) _lsp_send_headers( &es );
                if( pSettings->scripting.error_output_socket ) {
                    send_buffer_string( args->sendbuf, "luasp call error: " );
                    send_buffer_string( args->sendbuf, errmsg );
                }
                LOG_FILE( log_ERROR, "call: %s:%i, %s", ri->filename, lst.line, errmsg );
            }
        }
        /* if no http headers were sent, do it now... */
        if( !es.headers_sent ) _lsp_send_headers( &es );
        /* closing... */
        lua_close( L );
        if( es.session != NULL ) free( es.session );
        kvlist_free( es.headers );
    }
    else { /* file does not exist */
        send_buffer_error_info( args->sendbuf, ri->filename, HTTP_STATUS_NOT_FOUND, ri->http_version );
    }

    return 0;
}

#endif  /* LUA_SUPPORT */
