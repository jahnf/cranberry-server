/* cranberry-server. A small C web server application with lua scripting, 
 * session and sqlite support. https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
/* Lua server side scripting - modified version from luasp.org */

#include "config.h"
#if LUA_SUPPORT
#include "luasp_reader.h"

#include <lua.h>
#include <string.h>

#ifndef __cplusplus
    #ifdef _MSC_VER
    /* inline keyword is not available in Microsoft C Compiler */
    #define inline __inline
    #endif
#endif

/** States for the Lua server page file reader */
enum {
    LST_UNDEFINED = 0,
    LST_CHAR1,
    LST_CHAR2,
    LST_CHAR3,
    LST_CHAR4,
    LST_STMT1,
    LST_STMT2,
    LST_STMT3,
    LST_STMT12,
    LST_STMT13,
    LST_COMMENT1,
    LST_COMMENT2,
#ifndef LUASP_LEAVE_LF_UNTOUCHED
    LST_LF1,
    LST_LF2,
#else
    LST_LF1 = LST_CHAR1,
#endif
    LST_INIT = LST_CHAR1
};

/* lookup table taken from http://luasp.org */
static const unsigned char lsp_char_lookup[256] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x61, 0x62, 0x74, 0x6e, 0x76, 0x66, 0x72, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void luasp_state_init( luasp_state_t* lst )
{
    if( lst == NULL ) return;
    
    lst->dp = lst->dp_end = lst->dp_cur = NULL;
    lst->fp = NULL;
    lst->st = LST_INIT;
    lst->line          = 1; 
    lst->cur_line_echo = 0;
    lst->buf_offset    = 0;
}

/* add a character to the lua buffer when inside an echo call... */
inline static void _lsp_add_char( luasp_state_t *lst, const int ch )
{
    switch( lsp_char_lookup[ch] ) {
        case 0x00:
            lst->buf[lst->buf_offset++]=ch;
            break;
        case 0xff:
            lst->buf[lst->buf_offset++]='.';
            break;
        case 0x6e: /* add actual linefeed character, so that error messages from lua appear with
                      correct line numbers for lua/html files... */
            lst->buf[lst->buf_offset++]='\\';
            lst->buf[lst->buf_offset++]='\n';
            break;
        default:
            lst->buf[lst->buf_offset++]='\\';
            lst->buf[lst->buf_offset++]=lsp_char_lookup[ch];
            break;
    }
}

/* start an echo output in lua buffer*/
inline static void _lsp_beg_echo( luasp_state_t *lst )
{
    static const char echo_start[]="echo('";
    strncpy( (char*)lst->buf+lst->buf_offset, echo_start, sizeof(echo_start)-1 );
    lst->buf_offset += (sizeof(echo_start)-1);
}

/* end an echo output in lua buffer */
inline static void _lsp_end_echo( luasp_state_t *lst )
{
    static const char echo_end[]="')";
    strncpy( (char*)lst->buf+lst->buf_offset, echo_end, sizeof(echo_end)-1 );
    lst->buf_offset += (sizeof(echo_end)-1);
}

/* start an echo output of lua variables ( when '<?=' appears ) */
inline static void _lsp_beg_var_echo( luasp_state_t *lst )
{
    static const char echo_start[]="echo(";
    strncpy( (char*)lst->buf+lst->buf_offset, echo_start, sizeof(echo_start)-1 );
    lst->buf_offset += (sizeof(echo_start)-1);
}

/* end lua variable echo */
inline static void _lsp_end_var_echo( luasp_state_t *lst )
{
    lst->buf[lst->buf_offset++]=')';
}

/* lua reader function that is passed to lua_read...
 * fills buffer with lua code and returns..
 * every code not within code tags <? ?> is replaced and printed
 * out with an 'echo' call. */
const char* luasp_reader_file( lua_State* L , void *ud, size_t* size )
{
    luasp_state_t *lst = ud; /* user data */
    int ch;
    #ifndef LUASP_LEAVE_LF_UNTOUCHED
    int whitespace_count = 0;
    #endif

    lst->buf_offset = 0;

    while( lst->buf_offset < LUASP_BUFLEN ) {

        ch = fgetc( lst->fp );
        if( ch==EOF )   {
            switch(lst->st)
            {
            case LST_UNDEFINED:
            case LST_CHAR1:
#ifndef LUASP_LEAVE_LF_UNTOUCHED
            case LST_LF1:
            case LST_LF2:
#endif
                break;
            case LST_CHAR2:
                _lsp_end_echo( lst );
                break;
            default:
                /*  */
                break;
            }
            lst->st = LST_UNDEFINED;
            break;
        }

        switch( lst->st ) {
        case LST_CHAR1:
            if( ch=='<' )
                lst->st = LST_CHAR4;
            else {
                _lsp_beg_echo( lst );
                _lsp_add_char( lst, ch );
                lst->cur_line_echo = 1;
                lst->st = LST_CHAR2;
            }
            break;
        case LST_CHAR2:
            if( ch=='<' )
                lst->st= LST_CHAR3;
             else {
                _lsp_add_char( lst, ch );
                lst->cur_line_echo = 1;
             }
            break;
        case LST_CHAR3:
            if( ch=='?' ) {
                _lsp_end_echo( lst );
                lst->st = LST_STMT1;
            } else {
                lst->buf[lst->buf_offset++] = '<';
                _lsp_add_char( lst, ch );
                lst->cur_line_echo = 1;
                lst->st = LST_CHAR2;
            }
            break;
        case LST_CHAR4:
            if( ch=='?' )
                lst->st = LST_STMT1;
            else {
                _lsp_beg_echo( lst );
                lst->buf[lst->buf_offset++] = '<';
                _lsp_add_char( lst, ch );
                lst->cur_line_echo = 1;
                lst->st = LST_CHAR2;
            }
            break;
        case LST_STMT1:
            if( ch=='=' ) {
                _lsp_beg_var_echo( lst );
                lst->st = LST_STMT2;
            } else if( ch=='#' ) {
                lst->st = LST_COMMENT1;
            } else {
                lst->buf[lst->buf_offset++] = ch;
                lst->st = LST_STMT12;
            }
            break;
        case LST_STMT2:
            if(ch=='?')
                lst->st = LST_STMT3;
            else
                lst->buf[lst->buf_offset++] = ch;
            break;
        case LST_STMT3:
            if( ch=='>' ) {
                _lsp_end_var_echo( lst );
                lst->st = LST_LF1;
            } else if(ch=='?') {
                lst->buf[lst->buf_offset++] = '?';
            } else {
                lst->buf[lst->buf_offset++] = '?';
                lst->buf[lst->buf_offset++] = ch;
                lst->st = LST_STMT2;
            }
            break;
        case LST_STMT12:
            if( ch=='?' )
                lst->st= LST_STMT13;
            else
                lst->buf[lst->buf_offset++] = ch;
            break;
        case LST_STMT13:
        if( ch=='>' ) {
                lst->buf[lst->buf_offset++] = ' ';
                lst->st = LST_LF1;
            } else if(ch=='?') {
                lst->buf[lst->buf_offset++] = '?';
            } else {
                lst->buf[lst->buf_offset++] = '?';
                lst->buf[lst->buf_offset++] = ch;
                lst->st = LST_STMT12;
            }
            break;
        case LST_COMMENT1:
            if( ch=='?' )
                lst->st = LST_COMMENT2;
            break;
        case LST_COMMENT2:
            if( ch=='>' )
                lst->st = LST_LF1;
            else if( ch!='?' )
                lst->st = LST_COMMENT1;
            break;
#ifndef LUASP_LEAVE_LF_UNTOUCHED

        #define LSP_UNGETC(c) ungetc( c, lst->fp )
        case LST_LF1: /* to cut \r & \n following a closing ?> tag like php does */
            if( ch=='\r' )
                lst->st = LST_LF2;
            else if( ch==' ' ) {
                ++whitespace_count;
            } else {
                if( ch!='\n' ) {
                    LSP_UNGETC(ch);
                    while( whitespace_count ) {
                        LSP_UNGETC(' ');
                        --whitespace_count;
                    }
                } else {
                    lst->buf[lst->buf_offset++] = '\n';
                    if( lst->cur_line_echo )
                        LSP_UNGETC(ch);
                    whitespace_count = 0;
                }
                lst->st = LST_CHAR1;
            }
            break;
        case LST_LF2:
            if( ch != '\n' ) {
                LSP_UNGETC(ch);
                while( whitespace_count ) {
                    LSP_UNGETC(' ');
                    --whitespace_count;
                }
            } else {
                lst->buf[lst->buf_offset++] = '\n';
                if( lst->cur_line_echo )
                    LSP_UNGETC(ch);
                whitespace_count = 0;
            }
            lst->st = LST_CHAR1;
            break;
#endif
        } /*switch*/

        if( ch=='\n' ) {
            lst->cur_line_echo = 0;
            ++lst->line;
        }

    }
    *size = lst->buf_offset;
    return (const char*) lst->buf;
}

const char* luasp_reader_res( lua_State* L , void *ud, size_t* size )
{
    luasp_state_t *lst = ud;
    int ch;
    #ifndef LUASP_LEAVE_LF_UNTOUCHED
    int whitespace_count = 0;
    #endif

    lst->buf_offset = 0;

    while( lst->buf_offset < LUASP_BUFLEN ) {
        if( lst->dp_cur==lst->dp_end ) {
            switch(lst->st)
            {
            case LST_UNDEFINED:
            case LST_CHAR1:
#ifndef LUASP_LEAVE_LF_UNTOUCHED
            case LST_LF1:
            case LST_LF2:
#endif
                break;
            case LST_CHAR2:
                _lsp_end_echo( lst );
                break;
            default:
                /* luaL_lsp_error(L); */
                break;
            }
            lst->st = LST_UNDEFINED;
            break;
        }

        ch = *(lst->dp_cur++);

        switch( lst->st ) {
        case LST_CHAR1:
            if( ch=='<' )
                lst->st = LST_CHAR4;
            else {
                _lsp_beg_echo( lst );
                _lsp_add_char( lst, ch );
                lst->cur_line_echo = 1;
                lst->st = LST_CHAR2;
            }
            break;
        case LST_CHAR2:
            if( ch=='<' )
                lst->st= LST_CHAR3;
             else {
                _lsp_add_char( lst, ch );
                lst->cur_line_echo = 1;
             }
            break;
        case LST_CHAR3:
            if( ch=='?' ) {
                _lsp_end_echo( lst );
                lst->st = LST_STMT1;
            } else {
                lst->buf[lst->buf_offset++] = '<';
                _lsp_add_char( lst, ch );
                lst->cur_line_echo = 1;
                lst->st = LST_CHAR2;
            }
            break;
        case LST_CHAR4:
            if( ch=='?' )
                lst->st = LST_STMT1;
            else {
                _lsp_beg_echo( lst );
                lst->buf[lst->buf_offset++] = '<';
                _lsp_add_char( lst, ch );
                lst->cur_line_echo = 1;
                lst->st = LST_CHAR2;
            }
            break;
        case LST_STMT1:
            if( ch=='=' ) {
                _lsp_beg_var_echo( lst );
                lst->st = LST_STMT2;
            } else if( ch=='#' ) {
                lst->st = LST_COMMENT1;
            } else {
                lst->buf[lst->buf_offset++] = ch;
                lst->st = LST_STMT12;
            }
            break;
        case LST_STMT2:
            if(ch=='?')
                lst->st = LST_STMT3;
            else
                lst->buf[lst->buf_offset++] = ch;
            break;
        case LST_STMT3:
            if( ch=='>' ) {
                _lsp_end_var_echo( lst );
                lst->st = LST_LF1;
            } else if(ch=='?') {
                lst->buf[lst->buf_offset++] = '?';
            } else {
                lst->buf[lst->buf_offset++] = '?';
                lst->buf[lst->buf_offset++] = ch;
                lst->st = LST_STMT2;
            }
            break;
        case LST_STMT12:
            if( ch=='?' )
                lst->st= LST_STMT13;
            else
                lst->buf[lst->buf_offset++] = ch;
            break;
        case LST_STMT13:
        if( ch=='>' ) {
                lst->buf[lst->buf_offset++] = ' ';
                lst->st = LST_LF1;
            } else if(ch=='?') {
                lst->buf[lst->buf_offset++] = '?';
            } else {
                lst->buf[lst->buf_offset++] = '?';
                lst->buf[lst->buf_offset++] = ch;
                lst->st = LST_STMT12;
            }
            break;
        case LST_COMMENT1:
            if( ch=='?' )
                lst->st = LST_COMMENT2;
            break;
        case LST_COMMENT2:
            if( ch=='>' )
                lst->st = LST_LF1;
            else if( ch!='?' )
                lst->st = LST_COMMENT1;
            break;
#ifndef LUASP_LEAVE_LF_UNTOUCHED
    #define LSP_UNGETC_RES(c) --lst->dp_cur

        case LST_LF1: /* to cut \r & \n following a closing ?> tag like php does */
            if( ch=='\r' )
                lst->st = LST_LF2;
            else if( ch==' ' ) {
                ++whitespace_count;
            } else {
                if( ch!='\n' ) {
                    LSP_UNGETC_RES(ch);
                    while( whitespace_count ) {
                        LSP_UNGETC_RES(' ');
                        --whitespace_count;
                    }
                } else {
                    lst->buf[lst->buf_offset++] = '\n';
                    if( lst->cur_line_echo )
                        LSP_UNGETC_RES(ch);
                    whitespace_count = 0;
                }
                lst->st = LST_CHAR1;
            }
            break;
        case LST_LF2:
            if( ch != '\n' ) {
                LSP_UNGETC_RES(ch);
                while( whitespace_count ) {
                    LSP_UNGETC_RES(' ');
                    --whitespace_count;
                }
            } else {
                lst->buf[lst->buf_offset++] = '\n';
                if( lst->cur_line_echo )
                    LSP_UNGETC_RES(ch);
                whitespace_count = 0;
            }
            lst->st = LST_CHAR1;
            break;
#endif
        } /*switch*/

        if( ch=='\n' ) {
            lst->cur_line_echo = 0;
            ++lst->line;
        }

    }
    *size = lst->buf_offset;
    return (const char*) lst->buf;
}

#endif  /* LUA_SUPPORT */
