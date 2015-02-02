/** @file webthread.c
 *  @author Jahn Fuchs
 *
 */
#include "config.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock.h>
    #include <time.h>
    #define OFFT_FMT "%lu"
    #define OFFT_FMT_CAST
#else
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <fcntl.h>
    #define closesocket(s) close(s);
    #define SOCKET_ERROR -1
    #define OFFT_FMT "%ju"
    #ifdef _CYGWIN
        #define OFFT_FMT_CAST  (uintmax_t)
    #else
        #define OFFT_FMT_CAST
    #endif
#endif

#if DEFLATE_SUPPORT
    /* if compiling with deflate support */
    #define MINIZ_HEADER_FILE_ONLY
    #include "miniz.c"
#endif

#include "cfile.h"
#include "webthread.h"
#include "http_defines.h"
#include "http_request.h"
#include "http_time.h"
#include "server_commands.h"
#include "settings.h"
#if LUA_SUPPORT
    #include "luasp.h"
#endif
#include "log.h"
SETLOGMODULENAME("webthread")

/* embedded resources */
#include "cresource.h"

/* Buffer sizes */
#define SENDBUF_SIZE 8192
#define DEFLATE_BUFSIZE 2048

#define STR(x) #x

typedef struct __s_reg_thread reg_thread_t;
struct __s_reg_thread {
    c_thread thread_id;
    time_t start_time;
    reg_thread_t *next;
};

/* Server thread regitster struct */
typedef struct __s_thread_register thread_register_t;
struct __s_thread_register {
    c_mutex mutex_threadcount;
    unsigned long ulThreadCount;
    c_mutex mutex_threadlist;
    reg_thread_t *threads; // list of running threads
};

/* Server threads register initialization */
void * webthread_init() {
    thread_register_t* pThreadRegister =
            (thread_register_t*)malloc(sizeof(thread_register_t));
    pThreadRegister->threads = NULL;
    pThreadRegister->ulThreadCount = 0;
    cthread_mutex_init( &pThreadRegister->mutex_threadcount );
    cthread_mutex_init( &pThreadRegister->mutex_threadlist );
    return pThreadRegister;
}

/* Returns the number of running server threads */
unsigned long webthread_count(void *init_data) {
    unsigned long c;
    thread_register_t* pWebThreadData = (thread_register_t*)init_data;
    cthread_mutex_lock( &pWebThreadData->mutex_threadcount );
    c = pWebThreadData->ulThreadCount;
    cthread_mutex_unlock( &pWebThreadData->mutex_threadcount );
    return c;
}

/* Server threads deinitialization */
void webthread_free( void *init_data ) {
    unsigned int i; /* we want to be able to compile on non-C99 compilers */
    thread_register_t* pThreadRegister = (thread_register_t*)init_data;
    if( !pThreadRegister ) return;

    /* This function requires that no more threads are started
       and registered after the call to webthread_free */

    /* Give all server threads some time to finish */
    for( i=0; i < 30; ++i ) {
        if( webthread_count(init_data) == 0 ) break;
        cthread_sleep( 333 );
    }
    cthread_mutex_lock( &pThreadRegister->mutex_threadcount );
    cthread_mutex_lock( &pThreadRegister->mutex_threadlist );
    cthread_mutex_unlock( &pThreadRegister->mutex_threadcount );
    cthread_mutex_unlock( &pThreadRegister->mutex_threadlist );
    cthread_mutex_destroy( &pThreadRegister->mutex_threadcount );
    cthread_mutex_destroy( &pThreadRegister->mutex_threadlist );
    free( pThreadRegister );
}

/* Register a thread to the thread register */
static void register_thread( thread_arg_t *args ) {
    thread_register_t* pThreadRegister = (thread_register_t*)args->pDataSrvThreads;
    reg_thread_t* pRegThread = (reg_thread_t*)malloc( sizeof(reg_thread_t) );

    pRegThread->start_time = time(0);
    pRegThread->thread_id = cthread_self();

    cthread_mutex_lock( &pThreadRegister->mutex_threadlist );
    cthread_mutex_lock( &pThreadRegister->mutex_threadcount );

    pRegThread->next = pThreadRegister->threads;
    pThreadRegister->threads = pRegThread;
    ++pThreadRegister->ulThreadCount;

    cthread_mutex_unlock( &pThreadRegister->mutex_threadcount );
    cthread_mutex_unlock( &pThreadRegister->mutex_threadlist );
}

/* Unregister a thread from the thread register */
static void unregister_thread( thread_arg_t *args ) {
    thread_register_t* pThreadRegister = (thread_register_t*)args->pDataSrvThreads;
    reg_thread_t *iter, *current, *prev = 0;
    c_thread self = cthread_self();

    cthread_mutex_lock( &pThreadRegister->mutex_threadlist );

        iter = pThreadRegister->threads;
        while( iter ) {

            current = iter;
            iter = iter->next;

            if( cthread_equal(current->thread_id, self) ) {
                if( prev ) prev->next = iter;
                else pThreadRegister->threads = iter;

                cthread_mutex_lock( &pThreadRegister->mutex_threadcount );
                --pThreadRegister->ulThreadCount;
                cthread_mutex_unlock( &pThreadRegister->mutex_threadcount );

                free(current);
                break;
            }
            else {
                prev = current;
            }
        }

    cthread_mutex_unlock( &pThreadRegister->mutex_threadlist );
}

/* free a thread argument struct */
void free_thread_arg(thread_arg_t *arg)
{
    if( !arg ) return;
    free( arg->client_addr );
    free( arg );
}

/* Server thread that handles a request */
CTHREAD_RET webthread(CTHREAD_ARG data)
{
    char send_buffer[SENDBUF_SIZE];		/* Request send buffer memory */
    char small_string_buf[32];          /* Small string buffer */

    send_buffer_t sendbuf;				/* send buffer object */
    int ret_val = 0;					/* thread return value, 0 = SUCCESS */
    int srv_cmd = 0;

    http_req_info_t *req_info = NULL;
    thread_arg_t *args = (thread_arg_t*)data;
    server_settings_t *pSettings = (server_settings_t*)args->pSettings;
//    args->buf = buf;

    /* register thread at the thread register */
    register_thread( args );

    /* read the http_request, here we can use the send buffer for receiving */
    req_info = http_request_read( args, REQ_FILL_ALL, &ret_val, send_buffer, sizeof(send_buffer) );

    if( ret_val != RRT_OKAY ) {
        /* try to read (and ignore) the rest of post request data */
        if( req_info->post_info ) {
            if( req_info->post_info->content_length )
                http_request_recv_post_and_throw_away( args->fd, req_info, send_buffer, sizeof(send_buffer) );
            else
                http_request_recv_until_timeout_or_error( args->fd, send_buffer, sizeof(send_buffer) );
        }
    }

    /* init send buffer */
    send_buffer_init( &sendbuf, args->fd, send_buffer, SENDBUF_SIZE, SBF_NONE );
    args->sendbuf = &sendbuf;

    /* reply on errors */
    if( ret_val != RRT_OKAY ) {
        switch( ret_val ) {
        case RRT_ALLOCATION_ERROR:
            LOG_FILE( log_ERROR, "Memory allocation error while reading request" );
            send_buffer_error_info( args->sendbuf, req_info->filename, HTTP_STATUS_INTERNAL_SERVER_ERROR, req_info->http_version );
            break;
        case RRT_MISSING_CONTENT_LENGTH:
            LOG_FILE( log_WARNING, "Request is missing required content length" );
            send_buffer_error_info( args->sendbuf, req_info->filename, HTTP_STATUS_LENGTH_REQUIRED, req_info->http_version );
            break;
        case RRT_FORM_FIELD_SIZE_EXCEEDED:
            LOG_FILE( log_WARNING, "Form field size exceeded." );
            send_buffer_error_info( args->sendbuf, req_info->filename, HTTP_STATUS_REQUEST_ENT_TOO_LARGE, req_info->http_version );
            break;
        case RRT_SOCKET_TIMEOUT:
            LOG_FILE( log_WARNING, "Timeout during reading request." );
            send_buffer_error_info( args->sendbuf, req_info->filename, HTTP_STATUS_REQUEST_TIMEOUT, req_info->http_version );
            break;
        case RRT_HEADER_LINE_SIZE_EXCEEDED:
            LOG_FILE( log_WARNING, "Request-URI Too Long." );
            send_buffer_error_info( args->sendbuf, req_info->filename, HTTP_STATUS_REQUEST_URI_TOO_LONG, req_info->http_version );
            break;
        case RRT_MALFORMED_REQUEST:
            LOG_FILE( log_WARNING, "Malformed http request." );
            send_buffer_error_info( args->sendbuf, req_info->filename, HTTP_STATUS_BAD_REQUEST, req_info->http_version );
            break;
        default:
            LOG_FILE( log_ERROR, "Error during reading http request." );
            send_buffer_error_info( args->sendbuf, req_info->filename, HTTP_STATUS_INTERNAL_SERVER_ERROR, req_info->http_version );
            break;
        }
        goto clean_up_thread;
    }

    /* check if request type is supported */
    switch( req_info->req_method ) {
    case REQUEST_GET:
    case REQUEST_POST:
        /* only GET and POST are currently supported*/
        break;
    default: {
        kv_item *header = kvlist_new_item( HTTP_HEADER_CONTENT_LENGTH, "0" );
        send_buffer_http_header( args->sendbuf, HTTP_STATUS_METHOD_NOT_ALLOWED, NULL, req_info->http_version );
        LOG_FILE( log_ERROR, "Request type not supported (%d).", req_info->req_method );
        kvlist_free( header );
        goto clean_up_thread;
        break;
        }
    }

    /* check if request addresses a built in server command */
    if( (srv_cmd = is_server_command(args->pDataSrvCmds, req_info->filename)) ) {
        /* server command */
        server_command_run_by_id( args->pDataSrvCmds, srv_cmd, req_info, args );
    #if LUA_SUPPORT
    /* if scripting is enabled and the type requested type is a lua page */
    } else if( pSettings->scripting.enabled && req_info->scripting == SSS_LUA ) {
        luasp_process( req_info, args );
    #endif
    } else {  /* static content */
        cfile_stat_t st;

        /* check for embedded resources if not disabled */
        if( !pSettings->disable_er ) {
            /* look for filename in embedded resources */
            cresource_t *efile = get_cresource(req_info->filename);
            if( efile != NULL ) {
                /* TODO make it possible to send embedded resources with deflate
                 *  -> This is only good if the connection to the server is very slow
                 *  otherwise it might be faster to just send the data which is in memory already */
                kv_item *header = kvlist_new_item( HTTP_HEADER_CONTENT_TYPE, req_info->mimetype );
                sprintf( small_string_buf, OFFT_FMT, OFFT_FMT_CAST efile->size );

                header = kvlist_new_item_push_front( HTTP_HEADER_CONTENT_LENGTH, small_string_buf, header );
                header = kvlist_new_item_push_front( HTTP_HEADER_CACHE_CONTROL,
                        "max-age=" STR(EMBEDDED_RES_CACHE_AGE_MAX), header );

                send_buffer_http_header( args->sendbuf, HTTP_STATUS_OK, header, req_info->http_version );
                send_buffer_flush( args->sendbuf );
                send( args->fd, (const char*)efile->data, efile->size, 0);
                kvlist_free( header );

                goto clean_up_thread;
            }
        }

        /* check for the requested file in the www root dir */
        if( pSettings->wwwroot &&
                cfile_getstat( req_info->filename, &st ) == CFILE_SUCCESS ) {

            FILE *pFile;
            /* check if file is readable */
            if( st.type == CFILE_TYPE_REGULAR && (pFile = fopen(req_info->filename, "rb")) ) {
                int ret;
                kv_item *header = kvlist_new_item( HTTP_HEADER_CONTENT_TYPE, req_info->mimetype );
                /* with deflate support we compress static data of certain types */
                #if DEFLATE_SUPPORT
                const char *client_ae;
                if( pSettings->deflate &&  /* check if deflate is on in settings */
                    (req_info->mt_flags & MIMETYPE_FLAG_COMPRESSABLE) &&  /* check mimetype flags.. */
                    (client_ae = kvlist_get_value_from_key(HTTP_HEADER_ACCEPT_ENCODING, req_info->header_info))
                      && strstr(client_ae,"deflate") ) /* and check if client accepts deflate encoding */
                {
                    z_stream stream;
                    size_t infile_remaining = st.size;
                    unsigned char deflate_buf[DEFLATE_BUFSIZE];

                    header = kvlist_new_item_push_front( HTTP_HEADER_CONTENT_ENCODING, "deflate", header );
                    /* Static content can and should be cached by browsers or proxies. */
                    header = kvlist_new_item_push_front( HTTP_HEADER_CACHE_CONTROL,
                                       "max-age=" STR(STATIC_CACHE_AGE_MAX), header );

                    // Init the z_stream
                    memset( &stream, 0, sizeof(stream) );
                    stream.next_in = deflate_buf;
                    stream.avail_in = 0;
                    stream.next_out = (unsigned char*)send_buffer;
                    stream.avail_out = SENDBUF_SIZE;

                    /* Compression. (use init that does not sent zlib headers (IE can't handle it) */
                    if( mz_deflateInitwoHeader(&stream, pSettings->deflate) != Z_OK ) {
                        LOG( log_ERROR, "deflateInit() failed!\n" );
                        send_buffer_http_header( args->sendbuf, HTTP_STATUS_INTERNAL_SERVER_ERROR, header, req_info->http_version );
                    } else {
                        int status;
                        /* if http version is 1.1 we send the result in chunked transfer encoding,
                         * because we don't know the compressed size beforehand */
                        if( req_info->http_version == HTTP_VERSION_1_1 ) args->sendbuf->flags |= SBF_CHUNKED;
                        send_buffer_http_header( args->sendbuf, HTTP_STATUS_OK, header, req_info->http_version );
                        send_buffer_flush( args->sendbuf );
                        for( ; ; ) {
                            if( !stream.avail_in ) {
                                /* Input buffer is empty, so read more bytes from input file. */
                                int p = 0;
                                /* read data until we have WTBUFSIZE bytes or ret = 0 */
                                while( p < sizeof(deflate_buf) &&
                                        (ret = (int)fread( &deflate_buf[p], 1, sizeof(deflate_buf), pFile)) ) {
                                    p += ret;
                                }
                                stream.next_in = deflate_buf;
                                stream.avail_in = p;
                                infile_remaining -= p;
                            }

                            status = mz_deflate( &stream, infile_remaining ? Z_NO_FLUSH : Z_FINISH );

                            if( (status == Z_STREAM_END) || (!stream.avail_out) ) {
                                // Output buffer is full, or compression is done, so write buffer to output file.
                                args->sendbuf->curpos = SENDBUF_SIZE - stream.avail_out;
                                send_buffer_flush( args->sendbuf );
                                stream.next_out = (unsigned char*)send_buffer;
                                stream.avail_out = SENDBUF_SIZE;
                            }

                            if (status == Z_STREAM_END)
                                break;
                            else if (status != Z_OK) {
                                LOG( log_ERROR, "deflate() failed with status %i!", status );
                                break;
                            }
                        }
                        if( deflateEnd(&stream) != Z_OK ) {
                            LOG( log_ERROR, "deflateEnd() failed!" );
                        }
                    }

                } else
                #endif
                {
                    sprintf( small_string_buf, OFFT_FMT, OFFT_FMT_CAST st.size );
                    header = kvlist_new_item_push_front( HTTP_HEADER_CONTENT_LENGTH, small_string_buf, header );
                    /* Static content can and should be cached by browsers or proxies. */
                    header = kvlist_new_item_push_front( HTTP_HEADER_CACHE_CONTROL,
                                       "max-age=" STR(STATIC_CACHE_AGE_MAX), header );
                    send_http_header( args->fd, HTTP_STATUS_OK, header, req_info->http_version );
                    while (	(ret = (int)fread(send_buffer, 1, SENDBUF_SIZE, pFile)) ) {
                            ret = send( args->fd, send_buffer, ret, 0 );
                    }
                }
                kvlist_free( header );
                fclose( pFile );
            }
            else
                send_buffer_error_info( args->sendbuf, req_info->filename, HTTP_STATUS_FORBIDDEN, req_info->http_version );
        }
        else
            send_buffer_error_info( args->sendbuf, req_info->filename, HTTP_STATUS_NOT_FOUND, req_info->http_version );
    }

    /* ---------------------------------------------- */
    clean_up_thread:
    /* ---------------------------------------------- */
    send_buffer_flush_last( args->sendbuf );
    closesocket(args->fd);
    free_req_info(req_info);
    unregister_thread(args);
    free_thread_arg(args);

    return (CTHREAD_RETURN) (long) ret_val;
}
