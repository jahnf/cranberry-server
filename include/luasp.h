#ifndef _LUASP_H_
#define _LUASP_H_

#include "config.h"

#if LUA_SUPPORT

#include "webthread.h"
#include "http_request.h"

/** Initialize luasp module. */
void * luasp_init( thread_arg_t *wargs );

/** Deinitialize and free luasp. */
void luasp_free( void *data );

/** Luasp main function, tries to open the requested file and interpret
 * it as Lua server page. */
int luasp_process( http_req_info_t *ri, thread_arg_t *args );

#endif // LUA_SUPPORT
#endif // _LUASP_H_

