/** @file websession.c
 *	@author Jahn Fuchs
 *
 *  web session management
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "_stdint.h"
#include "webthread.h"
#include "websession.h"
#include "log.h"

SETLOGMODULENAME("websession");

typedef struct session_data_item_s session_data_item_t;
struct session_data_item_s {
    int id;
    void *data;
    session_data_free_fun free_function;
    session_data_item_t *next;
};

/* web session internal struct */
typedef struct _websession_internal_s websession_int_t;
struct _websession_internal_s {
    unsigned secret;                    /* secret session number */
    time_t valid_until;                 /* time until session is valid, if not updated */
    c_mutex data_mutex;
    session_data_item_t *session_data;  /* list of registered session data */
    websession_int_t *next;
};

/* web session register */
typedef struct {
    unsigned cleanup_counter;
	c_rwlock sess_lock;
	c_mutex sess_del_mutex;
    websession_int_t *sess_list;    /* pointer to first session */
    websession_int_t *sess_deleted; /* pointer to first del session */
} websession_register_t;

/**/
static void ** _ws_get_data(session_data_item_t *beg, const int id)
{
    session_data_item_t *it = beg;
    for( ; it; it = it->next ) {
        if( id == it->id )
            return &it->data;
    }
    return NULL;
}

void ** websession_register_data(session_t *session, const int id, void *data,
                                 session_data_free_fun free_fun)
{
    if( session && session->ref ) {
        websession_int_t *ref = session->ref;
        void **return_value = NULL;
        cthread_mutex_lock( &ref->data_mutex );
        /* if id not yet registered */
        if( !_ws_get_data(ref->session_data, id) ) {
            session_data_item_t *data_item = malloc(sizeof(session_data_item_t));
            if( data_item ) {
                data_item->id = id;
                data_item->data = data;
                data_item->free_function = free_fun;
                /* add to beginning of list */
                data_item->next = ref->session_data;
                ref->session_data = data_item;
                return_value = &data_item->data;
            }
        }
        cthread_mutex_unlock( &ref->data_mutex );
        return return_value;
    }
    return NULL;
}

void websession_unregister_data(session_t *session, const int id)
{
    if( session && session->ref ) {
        websession_int_t *ref = session->ref;
        session_data_item_t *it, *prev = NULL;
        cthread_mutex_lock( &ref->data_mutex );
        for( it = ref->session_data; it; prev = it, it = it->next ) {
            if( id == it->id ) {
                /* remove from list */
                if( prev == NULL )
                    ref->session_data = it->next;
                else
                    prev->next = it->next;
                cthread_mutex_unlock( &ref->data_mutex );
                /* and free */
                if( it->data && it->free_function )
                    it->free_function(it->data);
                free( it );
                return;
            }
        }
        cthread_mutex_unlock( &ref->data_mutex );
    }
}

void ** websession_get_data(session_t *session, const int id)
{
    if( session && session->ref ) {
        websession_int_t *ref = session->ref;
        return _ws_get_data(ref->session_data, id);
    }
    return NULL;
}

static void _websession_session_data_list_free( session_data_item_t* session_data )
{
    session_data_item_t *tmp_del;
    while( session_data ) {
        tmp_del = session_data;
        session_data = session_data->next;
        if( tmp_del->data && tmp_del->free_function )
            tmp_del->free_function(tmp_del->data);
        free( tmp_del );
    }
}

static void _websession_session_cleanup( websession_register_t *sess_settings, time_t time )
{
    websession_int_t *ws_prev = NULL, *ws_tmp;
    cthread_rwlock_write_wait( &sess_settings->sess_lock );
    ws_tmp = sess_settings->sess_list;
    while( ws_tmp ) {
        if( ws_tmp->valid_until < time ) {
            /* remove from session list */
            if( ws_prev ) ws_prev->next = ws_tmp->next;
            else sess_settings->sess_list = ws_tmp->next;
            /* put into deleted list */
            cthread_mutex_lock( &sess_settings->sess_del_mutex );
            ws_tmp->next = sess_settings->sess_deleted;
            sess_settings->sess_deleted = ws_tmp;
            ws_tmp->valid_until += 1200; /* live some more time in deleted list */
            cthread_mutex_unlock( &sess_settings->sess_del_mutex );
            /* adjust pointers */
            if( ws_prev ) ws_tmp =  ws_prev->next;
            else ws_tmp = sess_settings->sess_list;
        } else {
            ws_prev = ws_tmp;
            ws_tmp = ws_tmp->next;
        }
    }
    cthread_rwlock_write_post( &sess_settings->sess_lock );

    cthread_mutex_lock( &sess_settings->sess_del_mutex );
    ws_tmp = sess_settings->sess_list;
    ws_prev = NULL;
    while( ws_tmp ) {
        if( ws_tmp->valid_until < time ) {
            /* remove from deleted session list */
            if( ws_prev ) ws_prev->next = ws_tmp->next;
            else sess_settings->sess_deleted = ws_tmp->next;
            /* free memory */
            _websession_session_data_list_free( ws_tmp->session_data );
            free( ws_tmp );
            /* adjust pointers */
            if( ws_prev ) ws_tmp =  ws_prev->next;
            else ws_tmp = sess_settings->sess_list;
        } else {
            ws_prev = ws_tmp;
            ws_tmp = ws_tmp->next;
        }

    }
    cthread_mutex_unlock( &sess_settings->sess_del_mutex );
}

/* initialize web session register (e.g. mutexes) .. */
void * websession_init()
{
    websession_register_t *session_register = malloc( sizeof(websession_register_t) );
    session_register->cleanup_counter = 0;
    session_register->sess_deleted = NULL;
    session_register->sess_list = NULL;
    cthread_rwlock_init( &session_register->sess_lock, 20 );
    cthread_mutex_init( &session_register->sess_del_mutex );
    return session_register;
}

/* free web session register */
void websession_free( void *data )
{
    websession_register_t *sess_settings = data;
    websession_int_t *ws_tmp;
	if( !sess_settings ) return;
	cthread_mutex_lock( &sess_settings->sess_del_mutex );
    cthread_rwlock_write_wait( &sess_settings->sess_lock );
    while( sess_settings->sess_list ) {
 		ws_tmp = sess_settings->sess_list;
 		sess_settings->sess_list = sess_settings->sess_list->next;
        _websession_session_data_list_free( ws_tmp->session_data );
 		free( ws_tmp );
 	}
 	while( sess_settings->sess_deleted ) {
 		ws_tmp = sess_settings->sess_deleted;
 		sess_settings->sess_deleted = sess_settings->sess_deleted->next;
        _websession_session_data_list_free( ws_tmp->session_data );
 		free( ws_tmp );
 	}
	cthread_rwlock_write_post( &sess_settings->sess_lock );
	cthread_mutex_unlock( &sess_settings->sess_del_mutex );
	cthread_rwlock_destroy( &sess_settings->sess_lock );
	cthread_mutex_destroy( &sess_settings->sess_del_mutex );
	free( sess_settings );
}

/* destroy a websession, i.e. make it invalid */
int websession_destroy(void *data, session_t *session )
{
	if( data == NULL ) return 0;

    if( session != NULL && session->ref != NULL ) {
        websession_register_t *session_register = data;
        websession_int_t *int_session = session->ref;
        websession_int_t *ws_iter;

        cthread_rwlock_read_wait( &session_register->sess_lock );
        for( ws_iter = session_register->sess_list; ws_iter; ws_iter = ws_iter->next ) {
            if( ws_iter == int_session ) break;
        }
        cthread_rwlock_read_post( &session_register->sess_lock );
        if( ws_iter ) {
            cthread_rwlock_write_wait( &session_register->sess_lock );
            /* make session invalid */
            ws_iter->valid_until = 0;
            cthread_rwlock_write_post( &session_register->sess_lock );
            return 1;
        }
    }
	return 0;
}

/* start a new web session */
session_t * websession_start( void *data, const char* sid, unsigned ttl_sec )
{
    websession_register_t *session_register = data;
    websession_int_t *ws_iter, *ws_ptr = NULL;
    session_t *session = NULL;
	const time_t now = time(NULL);
	unsigned invalid_count = 0;
	int secret = 0;

    if( session_register == NULL ) return NULL;

    if( sid != NULL ) {
		if( strlen(sid) < 24 ) return NULL;
        sscanf( sid, "%16lX", (uint64_t*)&ws_ptr );
		secret = atoi( sid+16 );
	}

    cthread_rwlock_read_wait( &session_register->sess_lock );
    /* update time for existing session and collect invalid sessions on the way*/
    for( ws_iter = session_register->sess_list; ws_iter != NULL; ws_iter = ws_iter->next ) {
		if( ws_iter->valid_until >= now ) {
			if( ws_ptr == ws_iter && ws_ptr->secret == secret ) {
				ws_iter->valid_until = now + ttl_sec;  /*update time*/
				break;
			}
		} else ++invalid_count;
	}
    cthread_rwlock_read_post( &session_register->sess_lock );

    /* force clean up every xx created sessions without cleanup */
    if( invalid_count || ++session_register->cleanup_counter > 1023 ) {
        /* move timed out sessions to deleted session list
           and clean up deleted session list. */
        _websession_session_cleanup(session_register, now);
       session_register->cleanup_counter = 0;
    }

	if( !ws_iter ) { /* no valid session found */
		/* create new session */
        if( (ws_iter = malloc(sizeof(websession_int_t))) ) {
            ws_iter->valid_until = now + ttl_sec;
            ws_iter->session_data = NULL;
            srand( (unsigned int)now );
            cthread_mutex_init( &ws_iter->data_mutex );
            ws_iter->secret = rand() % 99999998 + 1;
            cthread_rwlock_write_wait( &session_register->sess_lock );
            /* put new session in front of list */
            ws_iter->next = session_register->sess_list;
            session_register->sess_list = ws_iter;
            cthread_rwlock_write_post( &session_register->sess_lock );
        } else {
            LOG(log_ERROR, "allocation error, cannot allocate new session");
        }
	}

    if( ws_iter != NULL) {
        if( NULL != (session = malloc( sizeof( session_t) )) ) {
            sprintf( session->sid, "%016lX%08u", (uint64_t)ws_iter, ws_iter->secret);
            session->ref = (void*)ws_iter;
        }
	}

    return session;
}
