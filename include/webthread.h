/** @file webthread.h
 *	@author Jahn Fuchs
 *
 *  $LastChangedDate: 2011-01-19 10:20:36 +0100 (Wed, 19 Jan 2011) $
 *  $LastChangedBy: Fuchs $
 */

#ifndef WEBTHREAD_H_
#define WEBTHREAD_H_

#include "config.h"

#include "cthreads.h"
#include "http_reply.h"

#define STATIC_CACHE_AGE_MAX 21600         /* 6 hours */
#define EMBEDDED_RES_CACHE_AGE_MAX 604800  /* 7 days */

/* Argument struct for each server thread */
typedef struct {
    // filled in by main server thread:
    int fd;				// open socket
    size_t hit;			// hit number
    char *client_addr;	// client network address
    int client_port;    // client port

    /* pointers to settings and initialization/runtime
       data from different server modules */
    void *pSettings;
    void *pDataSrvThreads;
    void *pDataSrvCmds;
    void *pDataSrvSessions;
    #if LUA_SUPPORT
        void *pDataLuaScripting;
    #endif

    // buffer, set by the request handling thread
    //char *buf;                // pointer to webthread buffer
    send_buffer_t *sendbuf;   // pointer to sendbuffer

} thread_arg_t;


CTHREAD_RET webthread( CTHREAD_ARG );

void * webthread_init( void );
void webthread_free( void *init_data );

#endif /* WEBTHREAD_H_ */
