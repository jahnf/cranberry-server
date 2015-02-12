#ifndef _HTTP_TIME_H_
#define _HTTP_TIME_H_

/** @defgroup http_time HTTP Time
 * HTTP time utilities.
 * @{
 * @file http_time.h Header file. 
 */
 
#include <time.h>

/** Fills the buffer pointed to by `buf` with the current GMT-time 
 * in http header format, the buffer needs to be at least 30 characters long. */
char * http_time_now( char *buf );

/** Fills the buffer pointed to by `buf` with the GMT-time of `time` 
 * in http header format, the buffer needs to be at least 30 characters long. */
char * http_time( char *buf, const time_t time );

/** Parses a given character string for a possible http date string,
 * set time_t value if http date was found.
 * See also http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.1
 * Returns 0 on success, error code else (i.e. if no valid http time format was found) */
int http_time_mktime( const char *timestr,  time_t *time );

/** @} */

#endif /* _HTTP_TIME_H_ */
