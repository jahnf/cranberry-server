#ifndef LUASP_TYPES_H_
#define LUASP_TYPES_H_
#include "config.h"

#if LUA_SUPPORT

#define LUASP_GLOB_USERDATA_NAME "__luasp_args_"
#define LUASP_ENV_VAR_NAME "env"
/* date in the past, that is sent to make cookies invalid
 * .. also, most important date in history of mankind ;) */
#define LUASP_DATE_IN_PAST "Fri, 01 Oct 1982 23:52:00 GMT"
/* maximum cache entry age in seconds*/
#define LUASP_CACHE_MAX_AGE 3600

#include "webthread.h"
#include "websession.h"
#include "http_request.h"

#include <stdio.h>

/* states for the lua server page file reader */
enum {
    LST_UNDEFINED = 0,
    LST_CHAR1,
    LST_CHAR2,
    LST_CHAR3,
    LST_CHAR4,
    LST_STMT1,
    LST_STMT2,
    LST_STMT3,
    LST_STMT12,
    LST_STMT13,
    LST_COMMENT1,
    LST_COMMENT2,
#ifndef LUASP_LEAVE_LF_UNTOUCHED
    LST_LF1,
    LST_LF2,
#else
    LST_LF1 = LST_CHAR1,
#endif
    LST_INIT = LST_CHAR1
};

#define LUASP_BUFLEN 1024

/* A state that is used as userdata and given as argument
 * to consecutive calls of luasp_reader_xxx */
typedef struct {
    /* for embedded resources */
    const unsigned char* dp;     /* input data */
    const unsigned char* dp_end; /* input data end */
    const unsigned char* dp_cur; /* input data cur pos */
    /* for files */
    FILE* fp;					/* input file */

    int st;						/* current state */
    int line;					/* file line number */
    int cur_line_echo;			/* was there an echo command on the current line? */

    unsigned char buf[LUASP_BUFLEN + 32];	// output buffer
    unsigned int buf_offset;

} luasp_state_t;

/* lua page state that is registered as global lua userdata
 * variable, so it can be used as information by custom
 * lua c functions like http_header and others */
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
