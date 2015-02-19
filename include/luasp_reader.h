#ifndef LUASP_READER_H_
#define LUASP_READER_H_

#include "config.h"

#if LUA_SUPPORT

#include <lua.h>
#include <stdio.h>

#define LUASP_BUFLEN 1024

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
void luasp_state_init( luasp_state_t* lst ); 

const char* luasp_reader_file( lua_State* L, void *ud, size_t* size );
const char* luasp_reader_res( lua_State* L, void *ud, size_t* size );

#endif
#endif
