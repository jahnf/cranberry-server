#include "config.h"
#if LUA_SUPPORT

#include "luasp_common.h"

#ifndef __cplusplus
    #ifdef _MSC_VER
    /* inline keyword is not available in Microsoft C Compiler */
    #define inline __inline
    #endif
#endif

/* helper function for setting a table string value
 * for the current top stack value */
inline void lua_set_tablefield_string( lua_State *L, const char* key, const char *value ) {
    lua_pushstring( L, key);
    if( !value )
        lua_pushstring( L, "" );
    else
        lua_pushstring( L, value );

    lua_settable(L, -3);
}

/* helper function for setting a table integer value
 * for the current top stack value */
inline void lua_set_tablefield_integer( lua_State *L, const char* key, const int value ) {
    lua_pushstring( L, key);
    lua_pushinteger( L, value );
    lua_settable(L, -3);
}

#endif
