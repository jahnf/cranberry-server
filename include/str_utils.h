/** @file str_utils.h
 *  @author Jahn Fuchs
 */

#ifndef STR_UTILS_H_
#define STR_UTILS_H_

#include <stdlib.h>

/** C linked list of c-character strings. */
typedef struct stringlist_t stringlist_t;
struct stringlist_t {
    char *str;
    stringlist_t *next;
};

stringlist_t* stringlist_new( const char* str );
stringlist_t* stringlist_back( stringlist_t* sl );
stringlist_t* stringlist_push_front( stringlist_t* sl, const char* str );
stringlist_t* stringlist_push_back( stringlist_t* sl, const char* str );
unsigned stringlist_len( const stringlist_t* sl );
void stringlist_free( stringlist_t* sl );

/** Trim a string on the right end (removes blanks). */
char *strtrim_right( char *p );
/** Trim a string on the right end (removes blanks). */
char *strtrim_right_e( char *beg, char *end );

/* url decoding */
size_t url_decode( char *dest, const char *src );
size_t url_decode_l( char *dest, const char *src, const size_t slen );

#endif /* STR_UTILS_H_ */
