/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
#ifndef LUASP_COMMON_H_
#define LUASP_COMMON_H_

#include "config.h"

#if LUA_SUPPORT

#include <lua.h>

/** Helper function for setting a table string value
 * for the current top stack value */
void lua_set_tablefield_string( lua_State *L, const char* key, const char *value );

/** Helper function for setting a table integer value
 * for the current top stack value */
void lua_set_tablefield_integer( lua_State *L, const char* key, const int value );

/** Helper function for setting a table boolean value
 * for the current top stack value */
void lua_set_tablefield_boolean( lua_State *L, const char* key, const int value );

#endif /* LUA_SUPPORT */
#endif /* LUASP_COMMON_H_ */
