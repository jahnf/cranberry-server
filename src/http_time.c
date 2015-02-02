#include "http_time.h"

#include <stdio.h>

static const char * _ymonths[] = {  "Jan","Feb","Mar","Apr","May","Jun",
									"Jul","Aug","Sep","Oct","Nov","Dec" };
static const char * _wkdays[] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };

// buf needs to point to a buffer with at least 30 chars, writes 29 char date string + trailing 0
char * http_time_now( char *buf ) {
	time_t rawtime = time( NULL );
	return http_time( buf, rawtime );
}

char * http_time( char *buf, const time_t time ) {

#ifdef _WIN32
	// only 'thread-safe' on windows:
    const struct tm *ti = gmtime( &time );
#else
	struct tm ti_;
	const struct tm *ti = &ti_;
	gmtime_r( &time, &ti_ );
#endif

	sprintf( buf, "%s, %02d %s %04d %02d:%02d:%02d GMT",
								_wkdays[ti->tm_wday],
								ti->tm_mday,
								_ymonths[ti->tm_mon],
								ti->tm_year + 1900,
								ti->tm_hour,
								ti->tm_min,
								ti->tm_sec
								);
	return buf;
}

// parses timestr for a possible http date string, and if successful
// fill time with the correct time_t value.
// see also http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.1
// return 0 on success, error code else.. (if http_time was not formatted right...)
int http_time_mktime( const char *timestr,  time_t *time )
{
    char mon_buf[4] = {0};
    struct tm ts;
    int ret;

    /* Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123 */
    ret = sscanf( timestr, "%*3s, %2d %3s %4d %2d:%2d:%2d GMT", &ts.tm_mday, mon_buf, &ts.tm_year,
                                                                &ts.tm_hour, &ts.tm_min, &ts.tm_sec );

    /* Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format */
	if( ret != 6 )
		ret = sscanf( timestr, "%*3s %3s %2d %2d:%2d:%2d %4d", mon_buf, &ts.tm_mday, &ts.tm_hour,
															   &ts.tm_min, &ts.tm_sec, &ts.tm_year );

    /* Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036 */
    if( ret != 6 ) {
        ret = sscanf( timestr, "%*s %2d-%3s-%2d %2d:%2d:%2d GMT", &ts.tm_mday, mon_buf, &ts.tm_year,
																	&ts.tm_hour, &ts.tm_min, &ts.tm_sec );
        ts.tm_year += 100; /* treat 2-digit year values as 20xx .. */
    } else
		ts.tm_year -= 1900;

    if( ret == 6 ) {
		ts.tm_mon = 0;
		for( ; ts.tm_mon < 12; ++ts.tm_mon ) {
			if( mon_buf[0] == _ymonths[ts.tm_mon][0]
			    && mon_buf[1] == _ymonths[ts.tm_mon][1]
			    && mon_buf[2] == _ymonths[ts.tm_mon][2] ) break;
		}
        if( ts.tm_mon < 12 ) { /* okay */
			if( time ) *time = mktime( &ts );
			return 0;
		}
	}
	if( time ) *time = 0;
	return -1;
}
