/* cranberry-server. A small C web server application with lua scripting, 
 * session and sqlite support. https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
/** @addtogroup str_utils
 * @{
 * @file str_utils.c Source file. 
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "str_utils.h"
#include "char_defines.h"

#ifndef __cplusplus
    #ifdef _MSC_VER
    /* inline keyword is not available in Microsoft C Compiler */
    #define inline __inline
    #endif
#endif

/* url decoding map */
static const unsigned short gsHexDecodeMapAscii[256] = {
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9, 256, 256,
    256, 256, 256, 256, 256,  10,  11,  12,  13,  14,  15, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256,  10,  11,  12,  13,  14,  15, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256
};

/* decode url encoded strings, destination and source can be the same string */
size_t url_decode( char *dest, const char *src )
{
    unsigned short ch;
    const char *deststart = dest;

    while( *src  ) {
        switch (*src) {
        case ASCII_PLUS:
            *dest++ = ASCII_SPACE;
            break;
        case ASCII_PERCENT:
            ch = (gsHexDecodeMapAscii[(unsigned short)(*(src + 1))] << 4) |
                    gsHexDecodeMapAscii[(unsigned short)(*(src + 2))];
            if (ch < 256) { /* if one of the hex chars is bad, d >= 256 */
                *dest++ = (char) ch;
                src += 2;
            } else {
                *dest++ = ASCII_PERCENT;
            }
            break;
        default:
            *dest++ = *src;
            break;
        }
        ++src;
    }
    *dest = 0;
    return (unsigned int) (dest-deststart);
}

size_t url_decode_l( char *dest, const char *src, const size_t slen )
{
    unsigned short ch;
    const char *deststart = dest;
    const char *src_end = src + slen;

    while( src < src_end  ) {
        switch (*src) {
        case ASCII_PLUS:
            *dest++ = ASCII_SPACE;
            break;
        case ASCII_PERCENT:
            if( src+2 < src_end ) {
                ch = (gsHexDecodeMapAscii[(unsigned short)(*(src + 1))] << 4) |
                        gsHexDecodeMapAscii[(unsigned short)(*(src + 2))];
                if (ch < 256) { /* if one of the hex chars is bad,  d >= 256 */
                    *dest++ = (char) ch;
                    src += 2;
                    break;
                }
            }
            *dest++ = ASCII_PERCENT;
            break;
        default:
            *dest++ = *src;
            break;
        }
        ++src;
    }
    *dest = 0;
    return (unsigned int) (dest-deststart);
}

stringlist_t * stringlist_new( const char *str )
{
    stringlist_t *sl = (stringlist_t*)malloc( sizeof(stringlist_t) );
    if( sl != NULL ) {
        sl->next = NULL;
        if( str != NULL ) {
            if( NULL != (sl->str = (char*)malloc( strlen(str)+1 )) )
                strcpy( sl->str, str );
        } else {
            sl->str = NULL;
        }
    }
    return sl;
}

stringlist_t * stringlist_back( stringlist_t *sl )
{
    while( sl ) {
        if( !sl->next ) return sl;
        sl = sl->next;
    }
    return NULL;
}

stringlist_t * stringlist_push_front( stringlist_t *sl, const char *str )
{
    stringlist_t *sl_new = stringlist_new( str );
    sl_new->next = sl;
    return sl_new;
}

stringlist_t *stringlist_push_back( stringlist_t *sl, const char *str )
{
    stringlist_t *sl_new = stringlist_new( str );
    stringlist_t *sl_back = stringlist_back( sl );
    if( sl_back )
        sl_back->next = sl_new;
    else
        return sl_new;

    return sl;
}

unsigned stringlist_len( const stringlist_t *sl )
{
    unsigned c = 0;
    while( sl ) {
        ++c;
        sl = sl->next;
    }
    return c;
}

void stringlist_free( stringlist_t *sl )
{
    stringlist_t *sl_tmp;
    while( sl ) {
        sl_tmp = sl;
        sl = sl->next;
        free( sl_tmp->str );
        free( sl_tmp );
    }
}

/* djb2: this algorithm (k=33) was first reported by dan bernstein many 
 * years ago in comp.lang.c. Another version of this algorithm 
 * (now favored by bernstein) uses xor: `hash(i) = hash(i - 1) * 33 ^ str[i];` 
 * the magic of number 33 (why it works better than many other constants, 
 * prime or not) has never been adequately explained.
 * see also http://www.cse.yorku.ca/~oz/hash.html */
unsigned long strhash(const char* s)
{
    int c;
    unsigned long hash = 5381;

    while((c = *s++)) /* djb2 hash function */
    {
        /* hash = hash * 33 ^ c */
        hash = ((hash << 5) + hash) ^ c;
    }
    return hash;
}

/* Returns a pointer to the first non-blank character in the given string. */
inline char *strtrim_left( char *p )
{
    for( ; isspace((int)*p); ++p );
    return p;
}

char *strtrim_right_end( char *beg, char *end )
{
    while( end > beg && isspace((int)*--end)) {
        *end = '\0';
    }
    return end;
}

void str_tolower( char *s )
{
    for( ; *s; ++s )
        *s = (char)tolower((int)*s);
}

/* modifies string, by adding a \0 character when only blanks would follow */
char *strtrim_right( char *p )
{
    unsigned char *end;
    size_t len;

    if( p && *p ) {
        len = strlen(p);
        end = (unsigned char*)(p + len-1);
        while ( len )
        {
            if( isspace(*end) )
            {
                *end = 0;
                --end;
                --len;
            }
            else
                break;
        }
    }
    return(p);
}

/* combines the results of strtim_left and strtrim_right... */
inline char * strtrim( char * p )
{
    return strtrim_right( strtrim_left(p) );
}

inline void strreverse(char* begin, char* end)
{
    char aux;
    while (end > begin)
        aux = *end, *end-- = *begin, *begin++ = aux;
}

/** @} */
