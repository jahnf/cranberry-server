#ifndef _LUASP_CACHE_H_
#define _LUASP_CACHE_H_
#if LUA_SUPPORT

#include "webthread.h"
#include "lua.h"

void *luasp_cache_init( thread_arg_t *pArgs );
void luasp_cache_free( void *pCache );


int luasp_cache_add_lua_State(void *pcache, lua_State *L, const char *name );

#endif // LUA_SUPPORT
#endif // _LUASP_CACHE_H_

