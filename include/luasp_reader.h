#ifndef LUASP_READER_H_
#define LUASP_READER_H_

#include "config.h"

#if LUA_SUPPORT

#include <lua.h>

const char* luasp_reader_file( lua_State* L , void *ud, size_t* size );
const char* luasp_reader_res( lua_State* L , void *ud, size_t* size );

#endif
#endif
