#ifndef LUASP_READER_H_
#define LUASP_READER_H_

#include "config.h"

#if LUA_SUPPORT

#include <lua.h>
#include <stdio.h>

#define LUASP_BUFLEN 1024

/** States for the Lua server page file reader */
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

/** A state that is used as userdata and given as argument
 * to consecutive calls of luasp_reader_xxx */
typedef struct {
    /* for embedded resources */
    const unsigned char* dp;     /**< Input data pointer */
    const unsigned char* dp_end; /**< Input data pointer end */
    const unsigned char* dp_cur; /**< Input data current position */
    /* for files */
    FILE* fp;                   /**< Input file */

    int st;                     /**< Current state */
    int line;                   /**< File line number */
    int cur_line_echo;          /**< Was there an echo command on the current line? */

    unsigned char buf[LUASP_BUFLEN + 32]; /**< output buffer */
    unsigned int buf_offset;              /**< buffer offset */
} luasp_state_t;

/** Initialize the state `lst` */
luasp_state_init( luasp_state_t* lst );

const char* luasp_reader_file( lua_State* L, void *ud, size_t* size );
const char* luasp_reader_res( lua_State* L, void *ud, size_t* size );

#endif
#endif
