/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
/** @file webthread.h */

#ifndef WEBTHREAD_H_
#define WEBTHREAD_H_

#include "config.h"

#include "cthreads.h"
#include "http_reply.h"

/** Caching age send to browser for static resources. */
#define STATIC_CACHE_AGE_MAX 21600         /* 6 hours */
/** Caching age send to browser for embedded static resources. */
#define EMBEDDED_RES_CACHE_AGE_MAX 604800  /* 7 days */

/** Argument struct for server threads */
typedef struct {
    /* filled in by main server thread: */
    int fd;             /**< open socket */
    size_t hit;         /**< hit number */
    char *client_addr;  /**< client network address */
    int client_port;    /**< client port */

    /* pointers to settings and initialization/runtime
       data from different server modules */
    void *pSettings;
    void *pDataSrvThreads;
    void *pDataSrvCmds;
    void *pDataSrvSessions;
    #if LUA_SUPPORT
        void *pDataLuaScripting;
    #endif

    /* buffer, set by the request handling thread */
    /* char *buf;                // pointer to webthread buffer*/
    
    send_buffer_t *sendbuf;   /**< pointer to send buffer */
} thread_arg_t;


CTHREAD_RET webthread( CTHREAD_ARG );

/** Initialize web thread module. */
void * webthread_init( void );

/** Deinitialize web thread module. */
void webthread_free( void *init_data );

#endif /* WEBTHREAD_H_ */
