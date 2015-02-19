/* cranberry-server. A small C web server application with lua scripting, 
 * session and sqlite support. https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
/** @file post_multipart.c */

#include <string.h>

#define HTTP_REQUEST_IMPL_
#include "http_request.h"
#include "char_defines.h"
#include "webthread.h"
#include "str_utils.h"

/* prerequisites: req_info->post_info is filled out with content length, 
 * boundary and buffer is starting with the http message body.
 * buffer length should be a reasonable size to fit in the mime lines like
 * content-disposition: ..... \r\n...
 */
int http_request_read_multipart_vars( thread_arg_t* args, http_req_info_t* req_info, unsigned int options )
{
    postdata_t* post_info = req_info->post_info;
    int endOfData = (post_info->bytes_read >= post_info->content_length) ? 1 : 0;
    char* pbuf = post_info->buf;
    /*size_t slen;*/

    /* check boundary */
    if( !post_info->mulpart_boundary || !post_info->mulpart_boundary[0] ) {
        return RRT_MALFORMED_REQUEST;
    }

    /* read */
    if( !endOfData && pbuf && pbuf[0] == 0 ) {
        int ret = _recv_data_timed_rrt( args->fd, &post_info->buf[0], post_info->buflen - 1, REQUEST_RECV_TIMEOUT );
        if( ret < 0 )
            return ret;
        else if( ret == 0 )
            return RRT_OKAY;

        post_info->buf[ret] = 0;
        post_info->bytes_read += ret;
    }

    #if 0
    /*TODO find first boundary, ignore everything before */
    while( pbuf && pbuf[0] ) {

      //char *pch = strstr(pbuf, "--");
        char *pch = strchr(pbuf, '-');

        if( pch == NULL ) {

            /* try to read more data from socket */
            slen = &buf[received] - pbuf;
            /* move data to beginning of buffer, include trailing 0 (slen+1) */
            memmove( &buf[0], pbuf, slen+1 );
            /* try to receive until buffer is full, timeout, socket error or until no more data can be received */
            /*received = http_request_recv_b( args->fd, &buf[slen], buflen-slen, REQUEST_RECV_TIMEOUT, &closed );*/
            /*received = _recv_data_timed( args->fd, &buf[slen], buflen-slen, REQUEST_RECV_TIMEOUT );*/
            received = _recv_data_timed_rrt( args->fd, &buf[slen], buflen-slen, REQUEST_RECV_TIMEOUT );
            if( received <= 0 ) {
              return received;
            }
            received += slen;
            buf[received] = 0;

            if( (pch = strstr( &buf[slen], ASCII_CRLF )) == NULL ) {
              if( err ) *err = RRT_MALFORMED_REQUEST;
              goto request_read_end;
            }
            pbuf = buf;
        }
        else if( pch == pbuf ) { /* end of header */
            pbuf += 2;  /* pbuf should now point to beginning of data or trailing \0 of header */
            break;
        }
    }
    #endif /*0*/

    /*TODO read one field after another.. */
    return RRT_OKAY;
}
