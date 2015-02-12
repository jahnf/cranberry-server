/** @file http_request_post.c
 *	@author Jahn Fuchs
 *
 *
 */

#include <string.h>

#define HTTP_REQUEST_IMPL_
#include "http_request.h"
#include "char_defines.h"
#include "str_utils.h"

int http_request_recv_post_and_throw_away(const int fd, http_req_info_t* req_info, char* buf, size_t buflen ) {
	if( req_info->post_info ) {
		int ret;
		while( req_info->post_info->bytes_read < req_info->post_info->content_length ) {
			ret = _recv_data_timed( fd, buf, (int)buflen, REQUEST_RECV_TIMEOUT );
			if(ret > 0 ) req_info->post_info->bytes_read += ret;
			else return ret;
		}
	}
	return 1;
}

#define POSTBUF_INCREMENTS MIN_POST_FORM_BUF_SIZE

int http_request_read_post_vars_urlencoded( const int fd, http_req_info_t *req_info ) {

	postdata_t *post_info = req_info->post_info;
	char *pch, *pbuf, *psearch, *rbuf;
	int EoD = ( post_info->bytes_read >= post_info->content_length )?1:0;
	int ret;
	kv_item *lcurr = NULL, *ltemp = NULL;

	pbuf = post_info->buf;
	
	if( !EoD && pbuf && pbuf[0] == 0 ) {
		ret = _recv_data_timed_rrt( fd, &post_info->buf[0], post_info->buflen-1, REQUEST_RECV_TIMEOUT );
		if( ret < 0 ) return ret;
		else if( ret == 0 ) return RRT_OKAY;
		
		post_info->buf[ret] = 0;
		post_info->bytes_read +=  ret;
	}

	while( pbuf && pbuf[0] ) {
		psearch = pbuf;
		// find '=', if not found try to receive more data from socket
		while( NULL == (pch = strchr( psearch, ASCII_EQUAL )) && !EoD ) {
			if( pbuf != post_info->buf ) {
				post_info->bufbytes = (unsigned)(&post_info->buf[post_info->bufbytes] - pbuf);
				memmove( &post_info->buf[0], pbuf, post_info->bufbytes + 1 );
				pbuf = &post_info->buf[0];
			}

			if( post_info->bufbytes >= post_info->buflen-1 ) {

				if( post_info->buflen < MAX_POST_FORM_FIELD_SIZE_KB * 1024 ) {
					size_t p = pbuf - post_info->buf;
					if( NULL == (rbuf = realloc( post_info->buf, post_info->buflen + POSTBUF_INCREMENTS )) ) {
						return RRT_ALLOCATION_ERROR;
					}
					post_info->buf = rbuf;
					pbuf = &post_info->buf[p];
					post_info->buflen += POSTBUF_INCREMENTS;
				}
				else
					break;
			}

			psearch = &post_info->buf[post_info->bufbytes];
			ret = _recv_data_timed( fd, psearch, post_info->buflen - 1 - post_info->bufbytes, REQUEST_RECV_TIMEOUT );
			if( ret < 0 ) {
				if( ret == RECV_SELECT_TIMEOUT ) return RRT_SOCKET_TIMEOUT;
				return RRT_SOCKET_ERR;
			}
			psearch[ret] = 0;
			post_info->bufbytes += ret;
			post_info->bytes_read += ret;

			if( post_info->bytes_read >= post_info->content_length ) EoD = 1;
		}

		if( pch == NULL )  { // buffer full, but no '=' found
			return RRT_FORM_FIELD_SIZE_EXCEEDED;
		}
		else if( pch == pbuf ) { // found '=' as first character
			return RRT_MALFORMED_REQUEST;
		}
		else {
			/* = found */

			pch[0] = 0;
			if( NULL == (ltemp = malloc(sizeof(kv_item))) )
				return RRT_ALLOCATION_ERROR;

			if(!lcurr) req_info->post_vars = ltemp;
			else lcurr->next = ltemp;
			lcurr = ltemp;
			lcurr->next = NULL;
			lcurr->value = NULL;

			if( NULL == (lcurr->key = malloc( pch-pbuf+1 )) ) {
				return RRT_ALLOCATION_ERROR;
			}
			url_decode( lcurr->key, pbuf );
			pbuf = pch + 1;
		}

		psearch = pbuf;
		// find '&' (if not try to receive more) or EndOfData
		while( NULL == (pch = strchr( psearch, ASCII_AMP )) && !EoD ) {

			if( pbuf != post_info->buf ) {
				post_info->bufbytes = (unsigned)(&post_info->buf[post_info->bufbytes] - pbuf);
				memmove( &post_info->buf[0], pbuf, post_info->bufbytes + 1 );
				pbuf = &post_info->buf[0];
			}

			if( post_info->bufbytes >= post_info->buflen-1 ) {
				if( post_info->buflen < MAX_POST_FORM_FIELD_SIZE_KB * 1024 ) {
					size_t p = pbuf - post_info->buf;
					
					if( NULL == (rbuf = realloc( post_info->buf, post_info->buflen + POSTBUF_INCREMENTS )) ) {
						return RRT_ALLOCATION_ERROR;
					}
					post_info->buf = rbuf;
					pbuf = &post_info->buf[p];
					post_info->buflen += POSTBUF_INCREMENTS;
				}
				else
					break;
			}

			psearch = &post_info->buf[post_info->bufbytes];
			ret = _recv_data_timed( fd, psearch, post_info->buflen - 1 - post_info->bufbytes, REQUEST_RECV_TIMEOUT );
			if( ret < 0 ) {
				if( ret == RECV_SELECT_TIMEOUT ) return RRT_SOCKET_TIMEOUT;
				return RRT_SOCKET_ERR;
			}
			psearch[ret] = 0;
			post_info->bufbytes += ret;
			post_info->bytes_read += ret;

			if( post_info->bytes_read >= post_info->content_length ) EoD = 1;
		}

		if( pch == NULL )  { // buffer full, but no '&' found
			if( EoD ) {	// end of data, we don't need to find a '&'
				if( NULL == (lcurr->value = malloc( &post_info->buf[post_info->bufbytes]-pbuf+1 )) )
					return RRT_ALLOCATION_ERROR;

				url_decode( lcurr->value, pbuf );
				pbuf = &post_info->buf[post_info->bufbytes];
			}
			else {	// end of data not reached, field value + field name too long for buffer
				return RRT_FORM_FIELD_SIZE_EXCEEDED;
			}
		}
		else {	// '&' found
			pch[0] = 0;
			if( NULL == (lcurr->value = malloc( pch-pbuf+1 )) )
				return RRT_ALLOCATION_ERROR;
			url_decode( lcurr->value, pbuf );
			pbuf = pch + 1;
		}
	}

	if( !EoD ) { // read rest of data
		while( post_info->bytes_read < post_info->content_length ) {
			ret = _recv_data_timed( fd, post_info->buf, post_info->buflen, REQUEST_RECV_TIMEOUT );
			if(ret >=0 ) post_info->bytes_read += ret;
			else if( ret == RECV_SELECT_TIMEOUT ) return RRT_SOCKET_TIMEOUT;
			else return RRT_SOCKET_ERR;
		}
	}
	
	return RRT_OKAY;
}
