/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
/** @file websession.h */

#ifndef WEBSESSION_H_
#define WEBSESSION_H_

#define WEBSESSION_COOKIE_NAME "WSESSID"

typedef struct {
    char sid[33];
    void *ref;
} session_t;

typedef void (*session_data_free_fun)(void *session_data);

/** Initialize a websession module. Returns pointer to the websession data. */
void * websession_init();

/** Uninitialize and free previous initialized websession modules. */
void websession_free( void *data );

/** Extends existing valid session with sid or creates a new one. */
session_t * websession_start( void *data, const char* sid, unsigned ttl_sec );

/** Makes previous created session invalid */
int websession_destroy( void *data, session_t *session );

/** Allows to register/attach data to a session with a given id.
 * @return NULL on failure.*/
void ** websession_register_data(session_t *session, const int id, void *data, session_data_free_fun free_fun);

/** Get web session data.
 * @return Pointer to registered data with requested id, NULL pointer if not found */
void ** websession_get_data(session_t *session, const int id);

/** Unregister and free previous attached data with a given id */
void websession_unregister_data(session_t *session, const int id);

#endif /* WEBSESSION_H_ */
