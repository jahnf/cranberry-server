#ifndef _HTTP_TIME_H_
#define _HTTP_TIME_H_

#include <time.h>

/** Fills buffer pointed to by buf with the current GMT time in http header format,
 * space in buffer needs to be at least 30 characters. */
char * http_time_now( char *buf );

/** fills buffer pointed to by buf with the GMT-time of time  in http header format,
 * space in buffer needs to be at least 30 characters. */
char * http_time( char *buf, const time_t time );

/** Parses a given character string for a possible http date string,
 * set time_t value if http date was found.
 * See also http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.1
 * Returns 0 on success, error code else (i.e. if no valid http time format was found) */
int http_time_mktime( const char *timestr,  time_t *time );

#endif /* _HTTP_TIME_H_ */
