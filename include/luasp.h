#ifndef _LUASP_H_
#define _LUASP_H_

#include "config.h"

#if LUA_SUPPORT

#include "webthread.h"
#include "http_request.h"

void * luasp_init( thread_arg_t *wargs );
void luasp_free( void *data );

int luasp_process( http_req_info_t *ri, thread_arg_t *args );

#endif // LUA_SUPPORT
#endif // _LUASP_H_

