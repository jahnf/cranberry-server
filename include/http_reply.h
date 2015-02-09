#ifndef HTTP_REPLY_H_
#define HTTP_REPLY_H_

#include "kvlist.h"

/** Send buffer flags. */
enum {
	SBF_NONE    = 0,             /**< No option. */
	SBF_CHUNKED = 1 << 0         /**< Chunked send buffer */
	// SBF_OTHERFLAG = 1 << 1,
	// SBF_ANOTHER   = 1 << 2,
};

/** Send buffer. */
typedef struct {
	int sockdesc;		/**< Socket descriptor. */
	char * buf;			/**< Pointer to buffer. */
	int flags;			/**< Buffer flags. */
	size_t bufsize;		/**< Buffer size. */
	size_t curpos;		/**< Current position in buffer. */
} send_buffer_t;

/** Returns pointer to a http status message for a given http status code. */
const char * get_statusmsg_from_http_status( const int http_status );

/** Initializes a send_buffer structure. */
void send_buffer_init( send_buffer_t *sendbuf, const int fd, char *buf, 
                       const size_t bufsize, const int flags );
                       
/** Flush send buffer (for in-between calls ). */
int send_buffer_flush( send_buffer_t *sendbuf );

/** Flush send buffer ( for last send (call this just before closing the socket) ) */
int send_buffer_flush_last( send_buffer_t *sendbuf );

/** Sends a simple http header with a content type field to a send buffer. */
void send_buffer_simple_http_header(send_buffer_t *sendbuf, const int http_status, 
                                    const char* content_type, const int version);
                                    
/** Sends a http header with all fields in header_info to a send buffer */
void send_buffer_http_header(send_buffer_t *sendbuf, const int http_status, 
                             const kv_item *header_info, const int version);
                             
/** Send data with given len to a send buffer */
void send_buffer_data(send_buffer_t *sendbuf, const void *data, const size_t len);

/** Send a character ch to a send buffer ( and translate from EBSDIC to ASCII on s390 ) */
void send_buffer_char(send_buffer_t *sendbuf, const char ch);

/** Send a character ch to a send buffer */
void send_buffer_data_char(send_buffer_t *sendbuf, const char ch);

/** Send a null terminated character string to a send buffer 
 * (and translate from ebsdic to ascii on s390) */
void send_buffer_string(send_buffer_t *sendbuf, const char * str);

/** Send a terminated character string to a send buffer
 * (and translate from ebsdic to ascii on s390) */
void send_buffer_string_data(send_buffer_t *sendbuf, char * str, size_t len);

/** Send a simple http error page including http header for the 
 * given http status to a send buffer. */
void send_buffer_error_info(send_buffer_t *sendbuf, const char *filename,
                            const int status, const int http_ver );

/** Escapes double quotes and newline with a backslash when adding to send buffer */
void send_buffer_json_ascii(send_buffer_t *sendbuf, const char *str);

/** Send http headers with all extra fields in header_info directly to socket fd */
int send_http_header(const int fd, const int http_status, 
                     const kv_item *header_info, const int version);

#endif /* HTTP_REPLY_H_ */
