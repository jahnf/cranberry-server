/* cranberry-server. A small C web server application with lua scripting, 
 * session and sqlite support. https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
#include "config.h"
#if LUA_SUPPORT

#include "luasp_common.h"

#ifndef __cplusplus
    #ifdef _MSC_VER
    /* inline keyword is not available in Microsoft C Compiler */
    #define inline __inline
    #endif
#endif

inline void lua_set_tablefield_string( lua_State *L, const char* key, const char *value ) 
{
    lua_pushstring(L, key);
    if( !value )
        lua_pushstring(L, "") ;
    else
        lua_pushstring(L, value);

    lua_settable(L, -3);
}

inline void lua_set_tablefield_integer( lua_State *L, const char* key, const int value ) 
{
    lua_pushstring(L, key);
    lua_pushinteger(L, value);
    lua_settable(L, -3);
}

inline void lua_set_tablefield_boolean( lua_State *L, const char* key, const int value ) 
{
    lua_pushstring(L, key);
    lua_pushboolean(L, value);
    lua_settable(L, -3);
}

#endif
