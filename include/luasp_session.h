#ifndef LUASP_SESSION_H_
#define LUASP_SESSION_H_

#include "config.h"

#if LUA_SUPPORT

#include <lua.h>

#define LUASP_SESSION_DATA_ID 5

/** C implementation of the luasp function session_var. */
int lsp_session_var( lua_State *L );

/** C implementation of the luasp function session_destroy. */
int lsp_session_destroy( lua_State *L );

/** C implementation of the luasp function session_start. */
int lsp_session_start( lua_State *L );

#endif
#endif
