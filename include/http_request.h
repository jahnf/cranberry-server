#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

#include "kvlist.h"
#include "webthread.h"

#ifdef _WIN32
    #include <winsock.h>
#else
    #define SOCKET_ERROR -1
#endif

/* The following defines affect the buffer sizes and memory usage: */
  /* Maxium total length (including \r\n) of each http header line */
  #define MAX_HTTP_HEADER_LINE           4096
  /* Maximum length in kilobytes of a x-www-form-urlencoded field
   * of a post request. This means that for instance the text in a textarea
   * field in a post request can have the maximum size of 64 kilobytes */
  #define MAX_POST_FORM_FIELD_SIZE_KB	64
  /* TODO refactor & explain */
  #define MIN_POST_FORM_BUF_SIZE        2048

/* Timeout in seconds when receiving data
 * TODO This should be a setting */
#define REQUEST_RECV_TIMEOUT		    10

enum _RECV_FUNC_RETURNS_ {
    RECV_SOCKET_ERROR = SOCKET_ERROR,
    RECV_SELECT_ERROR = -2,
    RECV_SELECT_TIMEOUT = -3
};

enum _REQUEST_READ_RETURNS {
    RRT_OKAY = 0,
    RRT_SOCKET_ERR,
    RRT_SELECT_ERR,
    RRT_SOCKET_TIMEOUT,
    RRT_HEADER_LINE_SIZE_EXCEEDED,
    RRT_FORM_FIELD_SIZE_EXCEEDED,
    RRT_MISSING_CONTENT_LENGTH,
    RRT_MALFORMED_REQUEST,
    RRT_ALLOCATION_ERROR,
    RRT_TE_NOT_SUPPORTED, // transfer encoding not supported
    RRT_CT_NOT_SUPPORTED, // content type not supported
    RRT_UNKNOWN_ERR
};

/* flags for changing the the request handling */
enum _REQUEST_READ_OPTION_FLAGS {
    /* parse url for get vars */
    REQ_READ_FLAG_FILL_GET_VARS    = 1 << 0,
    /* parse and fill post vars of application/x-www-form-urlencoded
     * or multipart form data post requests */
    REQ_READ_FLAG_FILL_POST_VARS   = 1 << 1,
    /* parse header info */
    REQ_READ_FLAG_FILL_HEADER_INFO = 1 << 2,
    /* parse cookies, this implies REQ_READ_FLAG_FILL_HEADER_INFO */
    REQ_READ_FLAG_FILL_COOKIES     = (1 << 3) | REQ_READ_FLAG_FILL_HEADER_INFO,
    /* all of the above */
    REQ_FILL_ALL = REQ_READ_FLAG_FILL_GET_VARS | REQ_READ_FLAG_FILL_HEADER_INFO |
                    REQ_READ_FLAG_FILL_POST_VARS | REQ_READ_FLAG_FILL_COOKIES
};

/* http request methods
 * - see also http://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html#sec9 */
enum _HTTP_REQUEST_METHODS {
    REQ_METHOD_UNKNOWN = 0,
    REQUEST_GET,
    REQUEST_POST,
    REQUEST_HEAD,   /* TODO support HEAD REQUESTS */
    REQUEST_PUT,    /* not supported (yet) */
    REQUEST_DELETE, /* not supported (yet) */
    REQUEST_LINK,   /* not supported (yet) */
    REQUEST_UNLINK  /* not supported (yet) */
};

enum __SSSCRIPTING_TYPES {
    SSS_NONE = 0,
    SSS_LUA = 1
};

/* flags for mimetypes */
enum _MIMETYPE_FLAGS {
    MIMETYPE_FLAG_NONE = 0,
    /* hint that data with this mimetype is usually good compressable */
    MIMETYPE_FLAG_COMPRESSABLE = 1 << 0
};

/* content types of http post requests */
enum _HTTP_POST_CONTENT_TYPES {
    REQ_POST_CONTENT_TYPE_UNKNOWN = 0,
    /* post data is x-www-form-urlencoded
     * - see also http://en.wikipedia.org/wiki/Application/x-www-form-urlencoded*/
    REQ_POST_CONTENT_TYPE_X_WWW_FORM,
    /* post data is multipart/form-data
     * - see also http://tools.ietf.org/html/rfc2388 */
    REQ_POST_CONTENT_TYPE_MULPART_FORM_DATA
};

/* flags for post request data */
enum _REQUEST_POST_FLAGS {
    REQ_POST_FLAG_TE_CHUNKED = 1 << 0  /* chunked transfer encoding */
};

/** TODO describe postdata_t */
typedef struct {
    size_t content_length;
    size_t bytes_read;
    char *mulpart_boundary;
    unsigned short content_type;	//one of _REQUEST_POST_CONTENT_TYPES
    unsigned flags;

    char *buf;                      /**< Pointer to buffer */
    unsigned int buflen;			/**< Size of buffer */
    unsigned int bufbytes;			/**< Number of bytes filled in buffer */

} postdata_t;

/** HTTP request info */
typedef struct {
    int   req_method;       /**< HTTP request method */
    char *filename;			/**< Requested filename */
    char *mimetype;			/**< Determined return mimetype, based on request data */
    unsigned mt_flags;		/**< Determined mimetype flags */

    short http_version;		/**< HTTP version of request */
    short scripting;		/**< which server side scripting should be used to process return request.. */

    kv_item *get_vars;		/**< The list of GET variables (key/values) */
    kv_item *post_vars;		/**< The list of POST variables (key/values) */

    postdata_t *post_info;  /** TODO describe */

    kv_item *header_info;   /**< The list of header fields and their values */
    kv_item *cookie_info;   /**< The list of cookies and their values */
} http_req_info_t;

/** Free a http_req_info_t struct object */
void free_req_info( http_req_info_t* req_info );

/** Read a ht */
http_req_info_t* http_request_read( thread_arg_t *args, const int flags, int* err, char* getbuf, size_t buflen );
int http_request_read_post_vars_urlencoded( const int fd, http_req_info_t* req_info );
int http_request_recv_post_and_throw_away( const int fd, http_req_info_t* req_info, char* buf, size_t buflen );
int http_request_recv_until_timeout_or_error( const int fd, char* buf, size_t buflen );

/** Takes a request type enum value and returns the corresponding character string. */
const char * http_request_type_to_str( const int type );

#ifdef HTTP_REQUEST_IMPL_
int _recv_data_timed( const int fd, char *buf, const int buflen, const unsigned int timeout_sec );
int _recv_data_timed_rrt( const int fd, char *buf, const int buflen, const unsigned int timeout_sec );
#endif

#endif /* HTTP_REQUEST_H_ */
