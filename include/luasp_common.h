#ifndef LUASP_COMMON_H_
#define LUASP_COMMON_H_

#include "config.h"

#if LUA_SUPPORT

#include <lua.h>

/* helper function for setting a table string value
 * for the current top stack value */
void lua_set_tablefield_string( lua_State *L, const char* key, const char *value );
/* helper function for setting a table integer value
 * for the current top stack value */
void lua_set_tablefield_integer( lua_State *L, const char* key, const int value );

#endif
#endif
