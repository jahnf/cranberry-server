/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
/** @file http_reply.c
 *
 */

#include <stdio.h>
#include <string.h>

#include "http_defines.h"
#include "http_reply.h"
#include "http_time.h"

#include "version.h"

#ifdef _WIN32
    #include <winsock.h>
#else
    #define SOCKET_ERROR -1
    #include <sys/socket.h>
#endif

#ifndef __cplusplus
    #ifdef _MSC_VER
    /* inline keyword is not available in Microsoft C Compiler */
    #define inline __inline
    #endif
#endif

#include "char_defines.h"

/* HTTP states to string mapping */
static const struct {
    const int status;
    const char * statmsg;
} _http_stat[] = {
    {HTTP_STATUS_OK, "OK"},
    {HTTP_STATUS_NOT_FOUND, "Not Found"},
    {HTTP_STATUS_FORBIDDEN, "Forbidden"},
    {HTTP_STATUS_REQUEST_ENT_TOO_LARGE, "Request Entity Too Large"},
    {HTTP_STATUS_LENGTH_REQUIRED, "Length Required"},
    {HTTP_STATUS_REQUEST_URI_TOO_LONG, "Request-URI Too Long"},
    {HTTP_STATUS_UNAUTHORIZED, "Unauthorized"},
    {HTTP_STATUS_BAD_REQUEST, "Bad Request"},
    {HTTP_STATUS_REQUEST_TIMEOUT, "Request Timeout"},
    {HTTP_STATUS_SEE_OTHER, "See Other"},
    {HTTP_STATUS_FOUND, "Found"},
    {HTTP_STATUS_INTERNAL_SERVER_ERROR, "Internal Server Error"},
    {HTTP_STATUS_METHOD_NOT_ALLOWED, "Method Not Allowed"},
    {HTTP_STATUS_NOT_MODIFIED, "Not Modified"},
    {HTTP_STATUS_NO_CONTENT, "No Content"},
    {HTTP_STATUS_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"},
    {0, 0}
};

const char* get_statusmsg_from_http_status( const int http_status ) 
{
    int i=0;
    for( ; _http_stat[i].statmsg != NULL; ++i) {
        if( http_status == _http_stat[i].status ) return _http_stat[i].statmsg;
    }
    return "";
}

int send_http_header(const int fd, const int http_status, 
                     const kv_item *header_info, const int version)
{
    char buf[384];
    send_buffer_t sendbuf;
    send_buffer_init( &sendbuf, fd, buf, sizeof(buf), SBF_NONE );
    send_buffer_http_header( &sendbuf, http_status, header_info, version );
    send_buffer_flush( &sendbuf );
    return 1;
}

void send_buffer_init( send_buffer_t *sendbuf, const int fd, 
                       char *buf, const size_t bufsize, const int flags )
{
    sendbuf->sockdesc = fd;
    sendbuf->buf = buf;
    sendbuf->flags = flags;
    sendbuf->bufsize = bufsize;
    sendbuf->curpos = 0;
}

inline static int _send_buffer_flush_internal( send_buffer_t *sendbuf )
{
    const int ret = send( sendbuf->sockdesc, sendbuf->buf, (int)sendbuf->curpos, 0);
    sendbuf->curpos = 0;
    return ret;
}

int send_buffer_flush( send_buffer_t *sendbuf )
{
    /* if the buffer is configured for chunked transfer encoding... */
    if( (sendbuf->flags & SBF_CHUNKED) && sendbuf->curpos ) {
        /* see also http://en.wikipedia.org/wiki/Chunked_transfer_encoding */
        char buf[16];
        int len = sprintf( buf, "%lX" ASCII_CRLF, (long unsigned int)sendbuf->curpos );
        len = send( sendbuf->sockdesc, buf, len, 0 );
        len += _send_buffer_flush_internal( sendbuf );
        len += send( sendbuf->sockdesc, ASCII_CRLF, sizeof(ASCII_CRLF)-1, 0 );
        return len;
    }
    else if( sendbuf->curpos )
        return _send_buffer_flush_internal( sendbuf );

    return 0;
}

int send_buffer_flush_last( send_buffer_t *sendbuf )
{
    int ret = send_buffer_flush( sendbuf );
    if( sendbuf->flags & SBF_CHUNKED ) {
        /* if we are sending with chunked transfer encoding,
         * send the last part of a chunked http message */
        ret += send( sendbuf->sockdesc, "0" ASCII_CRLF ASCII_CRLF, 5, 0 );
    }
    return ret;
}

void send_buffer_simple_http_header(send_buffer_t *sendbuf, const int http_status, 
                                    const char* content_type, const int version)
{
    const kv_item it = {HTTP_HEADER_CONTENT_TYPE, (char *)content_type, 0};
    send_buffer_http_header(sendbuf, http_status, &it, version);
}

void send_buffer_http_header(send_buffer_t *sendbuf, const int http_status,
                             const kv_item *header_info, const int version)
{
    char buf[150];
    int len;

    if( version == HTTP_VERSION_1_1 )
        len = sprintf( buf, HTTP_VER_STRING_1_1 " %d %s", http_status,
                       get_statusmsg_from_http_status(http_status) );
    else
        len = sprintf( buf, HTTP_VER_STRING_1_0 " %d %s", http_status,
                       get_statusmsg_from_http_status(http_status) );

    send_buffer_string_data( sendbuf, buf, len );

    /* always include Date header field */
    send_buffer_string_data( sendbuf, ASCII_CRLF HTTP_HEADER_DATE ": ",
                             4 + sizeof(HTTP_HEADER_DATE) - 1 );
    send_buffer_string_data( sendbuf, http_time_now( buf ), 29 );

    while( header_info ) {
        if( header_info->key ) {
            send_buffer_string_data( sendbuf, ASCII_CRLF, 2 );
            send_buffer_string_data( sendbuf, header_info->key, strlen(header_info->key) );
            send_buffer_string_data( sendbuf, ": ", 2 );
            if( header_info->value )
                send_buffer_string_data( sendbuf, header_info->value, strlen(header_info->value) );
        }
        header_info = header_info->next;
    }
    if( version == HTTP_VERSION_1_1 ) {

        /* since we don't support persistent connections, send Connection: close header for HTTP/1.1
         * see http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html (section 14.10) */
        send_buffer_string_data( sendbuf, ASCII_CRLF HTTP_HEADER_CONNECTION, 2 + sizeof(HTTP_HEADER_CONNECTION)-1 );
        send_buffer_string_data( sendbuf, ": close", 7 );

        /* -- optional: tell the browser that we don't support range requests */
        /*send_buffer_string_data( sendbuf, ASCII_CRLF HTTP_HEADER_ACCEPT_RANGES, 2 + sizeof(HTTP_HEADER_ACCEPT_RANGES)-1 );
          send_buffer_string_data( sendbuf, ": none", 6 ); */
    }
    if( sendbuf->flags & SBF_CHUNKED ) {
        send_buffer_string_data( sendbuf, ASCII_CRLF, 2 );
        send_buffer_string_data( sendbuf, HTTP_HEADER_TRANSER_ENCODING, sizeof(HTTP_HEADER_TRANSER_ENCODING)-1 );
        send_buffer_string_data( sendbuf, ": chunked" ASCII_CRLF ASCII_CRLF, 13 );
        /* send the header, chunked data is following. */
        _send_buffer_flush_internal( sendbuf );
        return;
    }
    send_buffer_string_data( sendbuf, ASCII_CRLF ASCII_CRLF, 4 );
}

void send_buffer_string_data(send_buffer_t *sendbuf, char * str, size_t len)
{
    size_t buf_avail = sendbuf->bufsize - sendbuf->curpos;

    while( len > buf_avail ) {
        memcpy(&sendbuf->buf[sendbuf->curpos], str, buf_avail);
        sendbuf->curpos += buf_avail;
        send_buffer_flush(sendbuf);
        str += buf_avail;
        len -= buf_avail;

        buf_avail = sendbuf->bufsize - sendbuf->curpos;
    }

    memcpy(&sendbuf->buf[sendbuf->curpos], str, len);
    sendbuf->curpos += len;
}

void send_buffer_string(send_buffer_t *sendbuf, const char * str)
{
    send_buffer_string_data( sendbuf, (char*)str, strlen( str ) );
}

void send_buffer_char(send_buffer_t *sendbuf, const char ch)
{
    if( !(sendbuf->curpos < sendbuf->bufsize) )
        send_buffer_flush(sendbuf);
    sendbuf->buf[sendbuf->curpos] = ch;
    ++sendbuf->curpos;
}

void send_buffer_data_char(send_buffer_t *sendbuf, const char ch)
{
    if( !(sendbuf->curpos < sendbuf->bufsize) )
        send_buffer_flush(sendbuf);
    sendbuf->buf[sendbuf->curpos] = ch;
    ++sendbuf->curpos;
}

void send_buffer_data(send_buffer_t *sendbuf, const void * data, const size_t len)
{
    send_buffer_string_data( sendbuf, (char*)data, len );
}

void send_buffer_json_ascii(send_buffer_t *sendbuf, const char *str)
{
    static const char escaped_quote[2] = { ASCII_BACKSLASH, ASCII_DBLQUOTE };
    static const char escaped_lf[2] = { ASCII_BACKSLASH, ASCII_N };

    const char *pch = str;
    while( *pch  ) {

        if( *pch == ASCII_DBLQUOTE) {
            send_buffer_data( sendbuf, str, pch-str );
            send_buffer_data( sendbuf, escaped_quote, 2 );
            str = pch + 1;
        }
        else if( *pch == ASCII_LF ) {
            send_buffer_data( sendbuf, str, pch-str );
            send_buffer_data( sendbuf, escaped_lf, 2 );
            str = pch + 1;
        }

        ++pch;
    }
    send_buffer_data( sendbuf, str, strlen(str) );
}

void send_buffer_error_info(send_buffer_t *sendbuf, const char *filename,
                            const int status, const int http_ver )
{
    char numbuf[12];
    const char *status_msg = get_statusmsg_from_http_status(status);
    send_buffer_simple_http_header( sendbuf, status, HTTP_CONTENT_TYPE_HTML, http_ver);
    send_buffer_string_data( sendbuf, "<html><head><title>", 19 );
    send_buffer_string_data( sendbuf, numbuf, sprintf( numbuf, "%d ", status ) );
    send_buffer_string( sendbuf, status_msg );
    send_buffer_string_data( sendbuf, "</title></head><body><h1>", 25 );
    send_buffer_string( sendbuf, numbuf );
    send_buffer_string( sendbuf, status_msg );
    send_buffer_string_data( sendbuf, "</h1>", 5 );

    if( filename ) {
        send_buffer_string_data( sendbuf, "<p>Requested file: <b>", 22);
        send_buffer_string( sendbuf, filename );
        send_buffer_string_data( sendbuf, "</b><p>", 7 );
    }
    send_buffer_string_data( sendbuf, "<hr><address>cranberry-server ", 30 );
    send_buffer_string( sendbuf, get_version_string() );
    send_buffer_string_data( sendbuf, "</address></body></html>", 24 );
}
