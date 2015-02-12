/** @file http_request.c
 *  @author Jahn Fuchs
 *
 */

#include "http_defines.h"
#define HTTP_REQUEST_IMPL_
#include "http_request.h"
#include "str_utils.h"
#include "kv_iter.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//int _recv_data_timed( const int fd, char *buf, const int buflen, const unsigned int timeout_sec );
//int _recv_data_timed_rrt( const int fd, char *buf, const int buflen, const unsigned int timeout_sec );


#ifdef _WIN32
    #include <winsock.h>
    #define strtoull _strtoui64
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
#else
    #include <sys/socket.h>
#endif

#ifndef __cplusplus
    #ifdef _MSC_VER
    /* inline keyword is not available in Microsoft C Compiler */
    #define inline __inline
    #endif
#endif

#include "char_defines.h"

#if LUA_SUPPORT
    static const char DEFAULT_URL_FILE[] = "index.lsp";
#else
    static const char DEFAULT_URL_FILE[] = "index.html";
#endif

/* Helper defines for creating static extensions/mime-type table */
#define _SL(str) str, sizeof(str)-1
#define _ssN(str) _SL(str), SSS_NONE
#define _ssL(str) _SL(str), SSS_LUA

static const struct {
    const char *ext;            /* File extension */
    const unsigned char len;    /* extension length */
    const char *mimetype;       /* MIME-type */
    const unsigned short mlen;  /* MIME-type string length */
    const unsigned short sss;   /* is the extension for server side scripting?
                                 * see __SSSCRIPTING_TYPES */
    const unsigned flags;       /* different flags of content type */
} extensions [] = {
    /* most likely used extensions up front */
    {_SL("html"),_ssN(HTTP_CONTENT_TYPE_HTML),  MIMETYPE_FLAG_COMPRESSABLE},
    {_SL("css"), _ssN(HTTP_CONTENT_TYPE_CSS),   MIMETYPE_FLAG_COMPRESSABLE},
    {_SL("js"),  _ssN(HTTP_CONTENT_TYPE_JAVASCRIPT), MIMETYPE_FLAG_COMPRESSABLE},
    {_SL("ico"), _ssN(HTTP_CONTENT_TYPE_ICO), MIMETYPE_FLAG_NONE},
    {_SL("png"), _ssN(HTTP_CONTENT_TYPE_PNG), MIMETYPE_FLAG_NONE},
    /* lua server pages */
    {_SL("lsp"), _ssL(HTTP_CONTENT_TYPE_LUASP), MIMETYPE_FLAG_COMPRESSABLE},
    {_SL("lua"), _ssL(HTTP_CONTENT_TYPE_LUA), MIMETYPE_FLAG_COMPRESSABLE},
    /* fewer used extensions later in list */
    {_SL("txt"), _ssN(HTTP_CONTENT_TYPE_TXT), MIMETYPE_FLAG_COMPRESSABLE},
    {_SL("jpg"), _ssN(HTTP_CONTENT_TYPE_JPEG), MIMETYPE_FLAG_NONE},
    {_SL("jpeg"),_ssN(HTTP_CONTENT_TYPE_JPEG), MIMETYPE_FLAG_NONE},
    {_SL("json"),_ssN(HTTP_CONTENT_TYPE_JSON), MIMETYPE_FLAG_COMPRESSABLE},
    {_SL("gif"), _ssN(HTTP_CONTENT_TYPE_GIF), MIMETYPE_FLAG_NONE},
    {_SL("zip"), _ssN(HTTP_CONTENT_TYPE_ZIP), MIMETYPE_FLAG_NONE},
    {_SL("tar"), _ssN(HTTP_CONTENT_TYPE_TAR), MIMETYPE_FLAG_NONE},
    {_SL("tgz"), _ssN(HTTP_CONTENT_TYPE_TGZ), MIMETYPE_FLAG_NONE},
    {_SL("tar.gz"),_ssN(HTTP_CONTENT_TYPE_TGZ), MIMETYPE_FLAG_NONE},
    {_SL("gz"),  _ssN(HTTP_CONTENT_TYPE_GZ), MIMETYPE_FLAG_NONE},
    {_SL("log"), _ssN(HTTP_CONTENT_TYPE_TXT), MIMETYPE_FLAG_COMPRESSABLE},
    {_SL("tif"), _ssN(HTTP_CONTENT_TYPE_TIFF), MIMETYPE_FLAG_NONE},
    {_SL("tiff"),_ssN(HTTP_CONTENT_TYPE_TIFF), MIMETYPE_FLAG_NONE},
    {_SL("swf"), _ssN(HTTP_CONTENT_TYPE_FLASH), MIMETYPE_FLAG_NONE},
    {_SL("htm"), _ssN(HTTP_CONTENT_TYPE_HTML), MIMETYPE_FLAG_COMPRESSABLE},
    {_SL("pdf"), _ssN(HTTP_CONTENT_TYPE_PDF), MIMETYPE_FLAG_NONE},
    {_SL("c"),   _ssN(HTTP_CONTENT_TYPE_C_CXX), MIMETYPE_FLAG_COMPRESSABLE},
    {_SL("cpp"), _ssN(HTTP_CONTENT_TYPE_C_CXX), MIMETYPE_FLAG_COMPRESSABLE},
    /* multimedia, video, audio */
    {_SL("avi"), _ssN(HTTP_CONTENT_TYPE_AVI), MIMETYPE_FLAG_NONE},
    {_SL("mpg"), _ssN(HTTP_CONTENT_TYPE_MPG), MIMETYPE_FLAG_NONE},
    {_SL("mkv"), _ssN(HTTP_CONTENT_TYPE_MKV), MIMETYPE_FLAG_NONE},
    {_SL("mks"), _ssN(HTTP_CONTENT_TYPE_MKV), MIMETYPE_FLAG_NONE},
    {_SL("mk3d"),_ssN(HTTP_CONTENT_TYPE_MKV), MIMETYPE_FLAG_NONE},
    {_SL("mpeg"),_ssN(HTTP_CONTENT_TYPE_MPG), MIMETYPE_FLAG_NONE},
    {_SL("mp3"), _ssN(HTTP_CONTENT_TYPE_MP3), MIMETYPE_FLAG_NONE},
    {_SL("ogg"), _ssN(HTTP_CONTENT_TYPE_OGG), MIMETYPE_FLAG_NONE},
    /* applications */
    {_SL("doc"), _ssN(HTTP_CONTENT_TYPE_DOC), MIMETYPE_FLAG_NONE},
    {_SL("docx"), _ssN(HTTP_CONTENT_TYPE_DOCX), MIMETYPE_FLAG_NONE},
    {_SL("xls"), _ssN(HTTP_CONTENT_TYPE_XLS), MIMETYPE_FLAG_NONE},

    {NULL,0,NULL,0,0,0} };

static const struct {
    const char *str;
    const int req_method;
} _req_types[] = {
        {"GET",     REQUEST_GET},
        {"POST",    REQUEST_POST},
        {"HEAD",    REQUEST_HEAD},
        {"PUT",     REQUEST_PUT},
        {"DELETE",  REQUEST_DELETE},
        {"LINK",    REQUEST_LINK},
        {"UNLINK",  REQUEST_UNLINK},
        {0,0}
};

static void free_post_info( postdata_t *pdi )
{
    if( !pdi ) return;
    free( pdi->mulpart_boundary );
    free( pdi->buf );
    free( pdi );
}

inline static void init_req_info(http_req_info_t * req_info)
{
    static const http_req_info_t trq = {0};
    *req_info = trq;
}

void free_req_info(http_req_info_t * req_info)
{
    if( req_info == NULL ) return;
    free(req_info->filename);
    free(req_info->mimetype);
    kvlist_free(req_info->get_vars);
    kvlist_free(req_info->post_vars);
    free_post_info( req_info->post_info );
    kvlist_free(req_info->header_info);
    kvlist_free(req_info->cookie_info);
    free(req_info);
}

/* Returns a url-decoded version of str */
/* IMPORTANT: make sure to free() the returned string after use */
inline static char *url_decode_new_l( const char *str, const size_t len )
{
    char *buf = (char*)malloc( len + 1 );
    if( buf ) url_decode_l( buf, str, len );
    return buf;
}

const char * http_request_type_to_str( const int type )
{
    int i;
    for( i=0; _req_types[i].str; ++i ) {
        if( _req_types[i].req_method == type  ) {
            return _req_types[i].str;
        }
    }
    return NULL;
}

static int _parse_url( char *str, size_t slen, http_req_info_t *reqinfo, const int flags )
{
    char *pch = str;
    kv_item * lcurr = NULL;
    kv_item * ltemp = NULL;

    if( str[0] == 0 || str[0] == ASCII_QMARK ) {
        if( (reqinfo->filename = (char*)malloc( sizeof(DEFAULT_URL_FILE) )) == NULL) {
            return RRT_ALLOCATION_ERROR;
        }
        strcpy( reqinfo->filename, DEFAULT_URL_FILE );
        if( str[0] == ASCII_QMARK ) ++pch;
        else pch = NULL;
    } else {
        if( (pch = strchr( str, ASCII_QMARK )) ) {
            *pch = 0;
            ++pch;
        }
        if( (reqinfo->filename = url_decode_new_l( str, (pch?pch-str:strlen(str)) )) == NULL )
            return RRT_ALLOCATION_ERROR;
    }

    /* Tokenize the arguments in the url */
    if( (flags & REQ_READ_FLAG_FILL_GET_VARS) && pch ) {

        kviter_t kvi;
        kviter_reset( &kvi, ASCII_AMP, ASCII_EQUAL, pch, slen );
        while( kviter_next(&kvi) ) {
            if( kvi.keylen > 0 ) {
                if( NULL == (ltemp = (kv_item*)malloc( sizeof(kv_item) )) ) {
                    return RRT_ALLOCATION_ERROR;
                }
                if(!lcurr) {
                    reqinfo->get_vars = ltemp;
                } else {
                    lcurr->next = ltemp;
                }
                lcurr = ltemp;
                lcurr->next = NULL;

                if( NULL == (lcurr->key = url_decode_new_l( kvi.key, kvi.keylen )) ) {
                    lcurr->value = NULL;
                    return RRT_ALLOCATION_ERROR;
                }

                if( kvi.vallen ) {
                    if( NULL == (lcurr->value = url_decode_new_l( kvi.val, kvi.vallen )) )
                        return RRT_ALLOCATION_ERROR;
                } else
                    lcurr->value = NULL;
            } /* end if kvi.keylen > 0 */
        } /* end while kviter_next.. */
    } /* end if flags & ... */

    /* try to get default mimetype automatically from file extension */
    if(reqinfo->filename) {
        size_t flen = strlen(reqinfo->filename);

        unsigned int i = 0;
        for( ; extensions[i].ext != 0; ++i ) {
            if( flen > extensions[i].len &&
                    reqinfo->filename[flen-extensions[i].len-1] == ASCII_DOT &&
                    !strncmp(&reqinfo->filename[flen-extensions[i].len], extensions[i].ext, extensions[i].len)) {
                reqinfo->mimetype = (char*)malloc( extensions[i].mlen + 1 );
                strcpy(reqinfo->mimetype, extensions[i].mimetype);
                reqinfo->scripting = extensions[i].sss;
                reqinfo->mt_flags = extensions[i].flags;
                break;
            }
        } /* end for */

        if( !reqinfo->mimetype ) { /* if no mimetype found assign default mimetype */
            reqinfo->mimetype = (char*)malloc( extensions[0].mlen + 1 );
            strcpy(reqinfo->mimetype, extensions[0].mimetype);
        }
    }

    return RRT_OKAY;
}

int _recv_data_timed( const int fd, char *buf, const int buflen, const unsigned int timeout_sec )
{
    int ret;
    struct timeval tv;
    fd_set addr_set;

    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    
    FD_ZERO( &addr_set );
    FD_SET( fd, &addr_set );

    ret = select( fd+1, &addr_set, NULL, NULL, &tv );

    if( ret == -1 ) {
        return RECV_SELECT_ERROR; 
    }
    else if( ret ) {
        /* FD_ISSET( fd, &addr_set ) should be true */
        return recv( fd, buf, buflen, 0 );
    }
    else
        return RECV_SELECT_TIMEOUT;
}


int _recv_data_timed_rrt( const int fd, char *buf, const int buflen, const unsigned int timeout_sec )
{
    int ret =_recv_data_timed( fd, buf, buflen, timeout_sec );
    switch( ret ) {
    case RECV_SELECT_TIMEOUT:
        return RRT_SOCKET_TIMEOUT;
    case RECV_SOCKET_ERROR:
        return RRT_SOCKET_ERR;
    case RECV_SELECT_ERROR:
        return RRT_SELECT_ERR;
    }
    return ret;
}

inline static int _parse_cookie_info( kv_item** root, const char* cstr, size_t slen )
{
    kv_item * lcurr = NULL;
    kv_item * ltemp = NULL;
    kviter_t kvi;
    kviter_reset( &kvi, ASCII_SEMICOLN, ASCII_EQUAL, cstr, slen );
    while( kviter_next_i(&kvi, 1, ASCII_SPACE) ) {
        if( kvi.keylen > 0 ) {
            if( NULL == (ltemp = (kv_item*)malloc( sizeof(kv_item) )) ) {
                return RRT_ALLOCATION_ERROR;
            }
            if(!lcurr) {
                *root = ltemp;
            } else {
                lcurr->next = ltemp;
            }
            lcurr = ltemp;
            lcurr->next = NULL;

            if( NULL == (lcurr->key = url_decode_new_l( kvi.key, kvi.keylen )) ) {
                lcurr->value = NULL;
                return RRT_ALLOCATION_ERROR;
            }

            if( kvi.vallen ) {
                if( NULL == (lcurr->value = url_decode_new_l( kvi.val, kvi.vallen )) )
                    return RRT_ALLOCATION_ERROR;
            } else
                lcurr->value = NULL;
        } /* end if kvi.keylen > 0 */
    } /* end while kviter_next.. */
    return RRT_OKAY;
}


http_req_info_t* http_request_read( thread_arg_t *args, const int flags, int *err, char* getbuf, size_t buflen )
{
    char *pbuf = NULL, *buf = NULL, *tmp = NULL;
    ssize_t received; unsigned int i; int j;
    http_req_info_t *reqinfo;
    kv_item * lcurr = NULL;
    kv_item * ltemp = NULL;
    size_t slen = 0;

    if( !( reqinfo = (http_req_info_t*)malloc(sizeof(http_req_info_t)) ) ) {
        /* allocation error */
        if( err ) *err = RRT_ALLOCATION_ERROR;
        return NULL;
    }
    init_req_info( reqinfo );

    /* allocate get buffer, if no buffer given */
    if( getbuf == NULL ) {
        /* allocate buffer + additional byte for 0 char */
        if( !( buf = (char*)malloc( MAX_HTTP_HEADER_LINE + 1 ) ) ) {
            /* allocation error */
            if( err ) *err = RRT_ALLOCATION_ERROR;
            return reqinfo;
        }
        buflen = MAX_HTTP_HEADER_LINE;
    } else {
        buf = getbuf;
        --buflen;
    }

    reqinfo->req_method = REQ_METHOD_UNKNOWN;
    if( err ) *err = RRT_OKAY;

    /* selected via select() by main thread, ready to recv... */
    received = recv( args->fd, buf, (int)buflen, 0 );
    if( received == SOCKET_ERROR ) {
        if( err ) *err = RRT_SOCKET_ERR;
        goto request_read_end;
    } else if( received == 0 ) {
        if( err ) *err = RRT_MALFORMED_REQUEST;
        goto request_read_end;
    } else if( received < 4 ) {
        /* try to receive more bytes */
        slen = received;
        received = _recv_data_timed_rrt( args->fd, &buf[slen], (int)(buflen-slen), REQUEST_RECV_TIMEOUT );
        if( received < 0 ) {
            if( err ) *err = (int)received;
            goto request_read_end;
        }
        received += slen;
    }

    if( received < 4 ) {
        if( err ) *err = RRT_MALFORMED_REQUEST;
        goto request_read_end;
    }

    buf[received] = 0;
    pbuf = buf;

    /* find first space in what should sth. like "GET /index.html HTTP/1.1" */
    if( NULL == (tmp = strchr(pbuf, ASCII_SPACE)) ) {
        if( err ) *err = RRT_MALFORMED_REQUEST;
        goto request_read_end;
    }
    *tmp = 0;

    /* check for request type in request type array */
    for( j=0; _req_types[j].str != 0; ++j ) {
        if( strcmp(_req_types[j].str, pbuf) == 0 ) {
            reqinfo->req_method = _req_types[j].req_method;
            break;
        }
    }

    /* now proceed with the requested url/file */
    pbuf = tmp+1;
    /* ignore first slash character of uri */
    if( pbuf[0] && pbuf[0] == ASCII_SLASH ) ++pbuf;

    /* find next space or \r.. */
    for( i=0; ; ++i ) {
        if( pbuf[i] == ASCII_SPACE || buf[i] == ASCII_CR ) {
            pbuf[i] = 0; break;
        }
        else if( !pbuf[i] ) { /* end of string before space or \r */
            /* each line in the request has to be smaller than bufsize.. */
            if( err ) *err = RRT_HEADER_LINE_SIZE_EXCEEDED;
            goto request_read_end;
        }
    }

    /* parse and decode requested url, tokenize get parameters */
    if( (j = _parse_url( pbuf, i, reqinfo, flags )) < 0 ) {
        if( err ) *err = j;
        goto request_read_end; 
    }

    /* check if protocol version was sent, default is set to HTTP/1.0 */
    pbuf = &pbuf[i+1];
    if( *pbuf != ASCII_LF ) {
        char *pch = strstr( pbuf, ASCII_CRLF );
        if( pch == NULL ) {
            if( err ) *err = RRT_MALFORMED_REQUEST;
            goto request_read_end;
        }
        *pch = 0;
        if( strcmp( pbuf, HTTP_VER_STRING_1_1) == 0 )
            reqinfo->http_version = HTTP_VERSION_1_1;
        pbuf = pch + 1;
    }
    ++pbuf;

    /* now pbuf should point to next line of http request or the trailing 0 character */
    if( err ) *err = RRT_OKAY;
    /* extract all other header informations into key/value pair list */
    while( pbuf && pbuf[0] ) {

        char *pch = strstr(pbuf, ASCII_CRLF);

        if( pch == NULL ) {
            /* try to read more data from socket */
            slen = &buf[received] - pbuf;
            /* move data to beginning of buffer, include trailing 0 (slen+1) */
            memmove( &buf[0], pbuf, slen+1 );
            /* try to receive until buffer is full, timeout, socket error or until no more data can be received */
            //received = http_request_recv_b( args->fd, &buf[slen], buflen-slen, REQUEST_RECV_TIMEOUT, &closed );
            //received = _recv_data_timed( args->fd, &buf[slen], buflen-slen, REQUEST_RECV_TIMEOUT );
            received = _recv_data_timed_rrt( args->fd, &buf[slen], (int)(buflen-slen), REQUEST_RECV_TIMEOUT );
            if( received < 0 ) {
                if( err ) *err = (int)received;
                goto request_read_end;
            }
            received += slen;
            buf[received] = 0;

            if( (pch = strstr( &buf[slen], ASCII_CRLF )) == NULL ) {
                if( err ) *err = RRT_MALFORMED_REQUEST;
                goto request_read_end;
            }
            pbuf = buf;
        }
        else if( pch == pbuf ) { // end of header
            pbuf += 2;  // pbuf should now point to beginning of data or trailing \0 of header
            break;
        }

        if(pch) {
            char *pdblp;
            
            pch[0] = 0;
            pdblp = strchr(pbuf, ASCII_DBLPOINT);

            if( pdblp ) {
                pdblp[0] = 0;

                ltemp = (kv_item*)malloc(sizeof(kv_item));
                if(!lcurr) {
                    reqinfo->header_info = ltemp;
                } else {
                    lcurr->next = ltemp;
                }
                lcurr = ltemp;
                lcurr->next = NULL;

                lcurr->key = (char*)malloc( strlen(pbuf)+1 );
                strcpy(lcurr->key, pbuf);

                pbuf = pdblp +1;
                if( pbuf[0]==ASCII_SPACE ) pbuf += 1;

                slen = strlen(pbuf);
                lcurr->value = (char*)malloc( slen+1 );
                strcpy(lcurr->value, pbuf);

                if( (flags & REQ_READ_FLAG_FILL_COOKIES) && strcasecmp(lcurr->key, "Cookie") == 0 ) {
                    _parse_cookie_info( &reqinfo->cookie_info, lcurr->value, slen );
                }

            }
            else { // No double point found
                if( err ) *err = RRT_MALFORMED_REQUEST;
                return reqinfo;
            }
            pbuf = pch + 2;
        }
    }


    if( reqinfo->req_method == REQUEST_POST ) {
        char *transfer_encoding = kvlist_get_value_from_key( HTTP_HEADER_TRANSER_ENCODING, reqinfo->header_info );
        char *content_type = kvlist_get_value_from_key( HTTP_HEADER_CONTENT_TYPE, reqinfo->header_info );
        char *content_length = kvlist_get_value_from_key( HTTP_HEADER_CONTENT_LENGTH, reqinfo->header_info );


        if( NULL == (reqinfo->post_info = (postdata_t*)malloc( sizeof(postdata_t) )) ) {
            if( err ) *err = RRT_ALLOCATION_ERROR;
            goto request_read_end;
        } else {
            static const postdata_t structnull = {0};
            *(reqinfo->post_info) = structnull;
        }

        reqinfo->post_info->bytes_read = reqinfo->post_info->bufbytes = (unsigned)(&buf[received] - pbuf);

        if( getbuf == NULL ) {
            reqinfo->post_info->buf = buf;
            reqinfo->post_info->buflen = (unsigned)buflen+1;
            memmove( &(reqinfo->post_info->buf[0]), pbuf, reqinfo->post_info->bufbytes + 1 );
        }
        else {
            reqinfo->post_info->buflen = 
                (reqinfo->post_info->bufbytes + 1 > MIN_POST_FORM_BUF_SIZE)
                    ?reqinfo->post_info->bufbytes + 1:MIN_POST_FORM_BUF_SIZE;
                    
            if( !( reqinfo->post_info->buf = (char*)malloc( reqinfo->post_info->buflen ) ) ) {
                if( err ) *err = RRT_ALLOCATION_ERROR;
                goto request_read_end;
            }
            
            memcpy( &(reqinfo->post_info->buf[0]), pbuf, reqinfo->post_info->bufbytes + 1 );
        }

        if( transfer_encoding ) {
            if( strcmp(transfer_encoding, "chunked") != 0 ) {
                /* transfer encoding not supported */
                /* only chunked is supported, also no mixed 
                 * transfer encodings.. e.g. gzip, chunked */
                if( err ) *err = RRT_TE_NOT_SUPPORTED;
                goto request_read_end;
            }
            else {
                /* chunked transfer encoding */
                reqinfo->post_info->flags |= REQ_POST_FLAG_TE_CHUNKED;
            }
        }

        if( !(reqinfo->post_info->flags & REQ_POST_FLAG_TE_CHUNKED) ) {
            if( content_length ) {
                reqinfo->post_info->content_length = (size_t) strtoull( content_length, NULL, 10 );
            }
            else {
                /* missing content-length, this server needs this header for 
                 * post requests, if transfer encoding is not chunked */
                if( err ) *err = RRT_MISSING_CONTENT_LENGTH;
                goto request_read_end;
            }
        }

        if( content_type ) {
            #define HTTP_MULFORM_STRLEN (sizeof(HTTP_CONTENT_TYPE_MULTIPART_FORM)-1)
            #define BOUNDARY_STR "boundary="
            #define BOUNDARY_STRLEN (sizeof(BOUNDARY_STR)-1)

            if( strncasecmp( HTTP_CONTENT_TYPE_MULTIPART_FORM, content_type, HTTP_MULFORM_STRLEN ) == 0 ) {
                /* for multipart/form-data, we need the boundary... */
                if( (content_type = strstr( &content_type[HTTP_MULFORM_STRLEN], BOUNDARY_STR )) ) {
                    content_type += BOUNDARY_STRLEN;
                    reqinfo->post_info->mulpart_boundary = (char*)malloc( strlen(content_type) + 1 );
                    /* TODO check for allocation error */
                    strcpy( reqinfo->post_info->mulpart_boundary, content_type );
                    reqinfo->post_info->content_type = REQ_POST_CONTENT_TYPE_MULPART_FORM_DATA;
                }
                else {
                    /* missing boundary */
                    if( err ) *err = RRT_MALFORMED_REQUEST;
                    goto request_read_end;
                }
            }
            else if( strcasecmp(content_type, HTTP_CONTENT_TYPE_POST_WWW_FORM) == 0) {
                reqinfo->post_info->content_type = REQ_POST_CONTENT_TYPE_X_WWW_FORM;
            }
            else {
                /* content-type not supported */
                if( err ) *err = RRT_CT_NOT_SUPPORTED;
                goto request_read_end;
            }
        } /* endif content_type */
        else {
            /* no content-type for post request given */
            if( err ) *err = RRT_MALFORMED_REQUEST;
            goto request_read_end;
        }

        /* if request = x www form & we want to fill out post_vars... */
        if( reqinfo->post_info->content_type == REQ_POST_CONTENT_TYPE_X_WWW_FORM && (flags & REQ_READ_FLAG_FILL_POST_VARS) ) {
            /* read url encoded post variables */
            if( (received = http_request_read_post_vars_urlencoded( args->fd, reqinfo )) != RRT_OKAY ) {
                if( err ) *err = (int)received;
                goto request_read_end;
            }
        } else if( reqinfo->post_info->content_type == REQ_POST_CONTENT_TYPE_MULPART_FORM_DATA ) {
            /* TODO: handle multipart form data post requests here?
             * we need to check for a valid session before allowing file upload
             * we need to fill out other post variables as usual */
            
        }

    } /* end if request type is POST */

    request_read_end:
    if( reqinfo->post_info ) {
        free( reqinfo->post_info->buf );
        reqinfo->post_info->buf = NULL;
        reqinfo->post_info->buflen = 0;
    } else {
        if( getbuf == NULL )
            free( buf );
    }
    return reqinfo;
}

int http_request_recv_until_timeout_or_error(const int fd, char *buf, size_t buflen )
{
    int ret;
    do {
        ret =_recv_data_timed( fd, buf, (int)buflen, 3 );
    } while( ret > 0 );

    if( ret == RECV_SELECT_TIMEOUT ) return RRT_SOCKET_TIMEOUT;
    if( ret == RECV_SOCKET_ERROR ) return RRT_SOCKET_ERR;
    return RRT_SELECT_ERR;
}
