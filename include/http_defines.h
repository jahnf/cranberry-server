
#ifndef HTTP_DEFINES_H_
#define HTTP_DEFINES_H_

// defines of most used HTTP status codes.
/// @{
#define HTTP_STATUS_OK 						200
#define HTTP_STATUS_NO_CONTENT 				204

#define HTTP_STATUS_FOUND					302
#define HTTP_STATUS_SEE_OTHER				303  // HTTP/1.1
#define HTTP_STATUS_NOT_MODIFIED			304

#define HTTP_STATUS_BAD_REQUEST				400
#define HTTP_STATUS_UNAUTHORIZED			401
#define HTTP_STATUS_FORBIDDEN				403
#define HTTP_STATUS_NOT_FOUND				404
#define HTTP_STATUS_METHOD_NOT_ALLOWED		405
#define HTTP_STATUS_REQUEST_TIMEOUT			408
#define HTTP_STATUS_LENGTH_REQUIRED			411
#define HTTP_STATUS_REQUEST_ENT_TOO_LARGE	413	// request entity too large
#define HTTP_STATUS_REQUEST_URI_TOO_LONG	414

#define HTTP_STATUS_INTERNAL_SERVER_ERROR 	500
#define HTTP_STATUS_VERSION_NOT_SUPPORTED	505
/// @}

// other defines
#define HTTP_HEADER_CONTENT_TYPE		"Content-Type"
#define HTTP_HEADER_SET_COOKIE			"Set-Cookie"
#define HTTP_HEADER_CONTENT_LENGTH		"Content-Length"
#define HTTP_HEADER_CACHE_CONTROL		"Cache-Control"
#define HTTP_HEADER_EXPIRES				"Expires"
#define HTTP_HEADER_PRAGMA				"Pragma"
#define HTTP_HEADER_IF_MODIFIED_SINCE	"If-Modified-Since"
#define HTTP_HEADER_TRANSER_ENCODING	"Transfer-Encoding"
#define HTTP_HEADER_CONTENT_ENCODING	"Content-Encoding"
#define HTTP_HEADER_CONNECTION			"Connection"
#define HTTP_HEADER_DATE				"Date"
#define HTTP_HEADER_ACCEPT_RANGES		"Accept-Ranges"
// Content-Disposition header can be used to 'force' a browser to open
// a save-as dialog for the retrieved file instead of showing it
// see http://www.w3.org/Protocols/rfc2616/rfc2616-sec19.html
// and also http://www.jtricks.com/bits/content_disposition.html
#define HTTP_HEADER_CONTENT_DISPOSITION	"Content-Disposition"

#define HTTP_HEADER_ACCEPT_ENCODING	"Accept-Encoding"

// content types
#define HTTP_CONTENT_TYPE_HTML           "text/html"
#define HTTP_CONTENT_TYPE_CSS            "text/css"
#define HTTP_CONTENT_TYPE_JAVASCRIPT	 "application/x-javascript"
#define HTTP_CONTENT_TYPE_JSON			 "application/json"
#define HTTP_CONTENT_TYPE_ICO			 "image/x-icon"
#define HTTP_CONTENT_TYPE_PNG			 "image/png"
#define HTTP_CONTENT_TYPE_TIFF			 "image/tiff"
#define HTTP_CONTENT_TYPE_TXT			 "text/plain"
#define HTTP_CONTENT_TYPE_JPEG			 "image/jpeg"
#define HTTP_CONTENT_TYPE_ZIP			 "application/zip"
#define HTTP_CONTENT_TYPE_GZ			 "application/x-gzip"
#define HTTP_CONTENT_TYPE_TAR			 "application/x-tar"
#define HTTP_CONTENT_TYPE_TGZ			 "application/x-compressed"
#define HTTP_CONTENT_TYPE_PDF			 "application/pdf"
#define HTTP_CONTENT_TYPE_FLASH			 "application/x-shockwave-flash"
#define HTTP_CONTENT_TYPE_C_CXX			 "text/x-c"
#define HTTP_CONTENT_TYPE_GIF			 "image/gif"
#define HTTP_CONTENT_TYPE_LUA            "text/x-lua"
#define HTTP_CONTENT_TYPE_LUASP			 "text/x-luasp"

#define HTTP_CONTENT_TYPE_AVI            "video/x-msvideo"
#define HTTP_CONTENT_TYPE_MPG            "video/mpeg"
#define HTTP_CONTENT_TYPE_MKV            "video/x-matroska"
#define HTTP_CONTENT_TYPE_MP3            "audio/mpeg"
#define HTTP_CONTENT_TYPE_OGG            "audio/ogg"

#define HTTP_CONTENT_TYPE_DOC            "application/msword"
#define HTTP_CONTENT_TYPE_DOCX           "application/vnd.openxmlformats-officedocument.wordprocessingml.document"
#define HTTP_CONTENT_TYPE_XLS            "application/vnd.ms-excel"


#define HTTP_CONTENT_TYPE_POST_WWW_FORM	 "application/x-www-form-urlencoded"
#define HTTP_CONTENT_TYPE_MULTIPART_FORM "multipart/form-data"

// versions
#define HTTP_VERSION_1_0	0
#define	HTTP_VERSION_1_1	1

#define HTTP_VER_STRING_1_1 "HTTP/1.1"
#define HTTP_VER_STRING_1_0 "HTTP/1.0"

#endif /* HTTP_DEFINES_H_ */
