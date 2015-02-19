/* cranberry-server. A small C web server application with lua scripting, 
 * session and sqlite support. https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
/** @file luasp_session.c 
 * Lua server side scripting - session functions. */

#include "config.h"
#if LUA_SUPPORT

#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include "luasp_types.h"
#include "luasp_common.h"
#include "luasp_session.h"
#include "settings.h"
#include "http_defines.h"
#include "kvlist.h"

#ifndef __cplusplus
    #ifdef _MSC_VER
    /* inline keyword is not available in Microsoft C Compiler */
    #define inline __inline
    #endif
#endif

inline static void lsp_session_data_free( void *data )
{
    kvlist_free(data);
}

/* C implementation of the luasp function session_start */
int lsp_session_start( lua_State *L )
{
    luasp_page_state_t *es;
    const int n = lua_gettop(L);
    int ttl = 1800;
    session_t *wsdata;
    const char *sid;

    if( n > 1 )
        luaL_error( L, "incorrect number of arguments" );

    lua_getglobal( L, LUASP_GLOB_USERDATA_NAME );
    es = lua_touserdata( L, -1 );


    if( n == 1 && lua_isnumber(L,1) ) {
        ttl = lua_tointeger( L, 1 );
    } else {
        ttl = ((server_settings_t*)es->args->pSettings)->scripting.session_timeout;
    }

    sid = kvlist_get_value_from_key( WEBSESSION_COOKIE_NAME, es->ri->cookie_info );

    /*if( !sid ) sid = kvlist_get_value_from_key( WEBSESSION_FIELD_NAME, es->ri->get_vars );
      if( !sid ) sid = kvlist_get_value_from_key( WEBSESSION_FIELD_NAME, es->ri->post_vars ); */

    wsdata = websession_start( es->args->pDataSrvSessions, sid, ttl );

    if( wsdata ) {
        void **pp_data = websession_get_data( wsdata, LUASP_SESSION_DATA_ID );

        if( !pp_data ) {
            pp_data = websession_register_data( wsdata, LUASP_SESSION_DATA_ID,
                                                NULL, lsp_session_data_free);
        }

        if( pp_data ) {
            es->sess_vars = (kv_item**) pp_data;
        }

        if( es->sess_vars && *(es->sess_vars) ) {
            kv_item *iter = *(es->sess_vars);
            lua_getglobal( L, LUASP_ENV_VAR_NAME );
            lua_getfield(L, -1, "session");
            while( iter ) {
                lua_set_tablefield_string( L, iter->key, iter->value );
                iter = iter->next;
            }
        }
        /* add set-cookie header field */
        if( !es->headers_sent ) {
            kv_item *exists = kvlist_find_key( HTTP_HEADER_SET_COOKIE, es->headers );
            if( !exists )
                es->headers = exists = kvlist_new_item_push_front( HTTP_HEADER_SET_COOKIE, NULL, es->headers );
            if( exists ) {
                exists->value = realloc( exists->value,
                                         sizeof(wsdata->sid) + 1 + sizeof( WEBSESSION_COOKIE_NAME) + 2 + 29 + 8);
                /* TODO check for allocation error */
                if( ttl >= 0)
                    sprintf( exists->value, WEBSESSION_COOKIE_NAME "=%s; Max-Age=%d", wsdata->sid, ttl );
                else
                    sprintf( exists->value, WEBSESSION_COOKIE_NAME "=%s; Expires=" LUASP_DATE_IN_PAST, wsdata->sid );
            }
        }
        es->session = wsdata;
        /* return value */
        lua_pushstring( L, wsdata->sid );
    }
    else
        lua_pushnil( L );

    return 1;
}

/* C implementation of the luasp function session_var */
int lsp_session_var( lua_State *L )
{
    luasp_page_state_t *es;
    const int n = lua_gettop(L);

    if( n < 1 || n > 2 ) {
        luaL_error( L, "incorrect number of arguments" );
        return 0;
    }

    lua_getglobal( L, LUASP_GLOB_USERDATA_NAME );
    es = lua_touserdata( L, -1 );

    if( n == 1 ) {
        const char *field = lua_tostring( L, 1 );
        kv_item *exists = NULL;
        if( es->session && es->sess_vars )
            exists = kvlist_find_key( field, *(es->sess_vars) );
        if( exists && exists->value )
            lua_pushstring( L, exists->value );
        else
            lua_pushnil( L );
        return 1;
    } else {
        size_t vlen; kv_item *exists;
        const char *field = lua_tostring( L, 1 );
        const char *value = lua_tolstring( L, 2, &vlen );

        if( es->session && es->sess_vars ) {
            exists = kvlist_find_key( field, *(es->sess_vars) );

            if( lua_isnil(L,2) ) {
                /* unset lua session variable */
                lua_getglobal( L, LUASP_ENV_VAR_NAME );
                lua_getfield(L, -1, "session");
                lua_pushnil( L );
                lua_setfield( L, -2, field );

                if( exists ) {  /* remove from session var list in C */
                    kvlist_remove_item( es->sess_vars, exists );
                    exists = NULL;
                }
            } else if( exists ) {
                if( exists->value != NULL ) free( exists->value );
                exists->value = malloc( vlen+1 );
                strcpy( exists->value, value );
            } else {
                exists = *(es->sess_vars) = kvlist_new_item_push_front( field, value, *(es->sess_vars) );
            }
            if( exists && exists->value ) {
                lua_getglobal( L, LUASP_ENV_VAR_NAME );
                lua_getfield(L, -1, "session");
                lua_set_tablefield_string( L, exists->key, exists->value );
                lua_pushstring( L, exists->value );
            }
            else
                lua_pushnil( L );
            return 1;
        }
    }
    return 0;
}

/* C implementation of the luasp function session_destroy */
int lsp_session_destroy( lua_State *L )
{
    luasp_page_state_t *es;
    if( lua_gettop( L ) )
        luaL_error( L, "incorrect number of arguments" );

    lua_getglobal( L, LUASP_GLOB_USERDATA_NAME );
    es = lua_touserdata( L, -1 );

    if( es->session ) {
        const int bDestroyed = websession_destroy( es->args->pDataSrvSessions, es->session );
        if( bDestroyed ) { /* replace env.session with an empty table */
            lua_getglobal( L, LUASP_ENV_VAR_NAME );
            lua_newtable( L );
            lua_setfield( L, -2, "session" );
        }

        /* add set-cookie header field */
        if( !es->headers_sent ) {
            kv_item *exists = kvlist_find_key( HTTP_HEADER_SET_COOKIE, es->headers );
            if( !exists )
                es->headers = exists = kvlist_new_item_push_front( HTTP_HEADER_SET_COOKIE, NULL, es->headers );
            if( exists ) {
                if( exists->value != NULL ) free( exists->value );
                exists->value = malloc( 1 + sizeof(WEBSESSION_COOKIE_NAME) + 2 + 29 + 8);
                sprintf( exists->value, WEBSESSION_COOKIE_NAME "=; Expires=" LUASP_DATE_IN_PAST );
            }
        }

        lua_pushboolean( L, bDestroyed );
    }
    else
        lua_pushboolean( L, 0 );

    return 1;
}

#endif  /* LUA_SUPPORT */
