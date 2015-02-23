/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
#ifndef LUASP_TYPES_H_
#define LUASP_TYPES_H_
#include "config.h"

#if LUA_SUPPORT

#define LUASP_GLOB_USERDATA_NAME "__luasp_args_"
#define LUASP_ENV_VAR_NAME "env"

/* Date in the past, that is sent to make cookies invalid
 * .. also, most important date in history of mankind ;) */
#define LUASP_DATE_IN_PAST "Fri, 01 Oct 1982 23:52:00 GMT"

/* Maximum cache entry age in seconds*/
#define LUASP_CACHE_MAX_AGE 3600

#include "webthread.h"
#include "websession.h"
#include "http_request.h"

/** Lua page state that is registered as global Lua userdata
 * variable, so it can be used as information by custom
 * Lua c functions like http_header and others */
typedef struct {
    thread_arg_t *args;
    http_req_info_t *ri;
    int headers_sent;
    int http_status_code;
    kv_item *headers;
    session_t *session;
    kv_item **sess_vars;
} luasp_page_state_t;

#endif
#endif
