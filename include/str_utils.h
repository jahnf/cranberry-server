#ifndef STR_UTILS_H_
#define STR_UTILS_H_

/** @defgroup str_utils String Utils
 * Different C-string utilities.
 * @{
 * @file str_utils.h Header file. 
 */

#include <stdlib.h>

/** C linked list of c-character strings. */
typedef struct stringlist_t stringlist_t;

/** C linked list of c-character strings. */
struct stringlist_t {
    char *str;
    stringlist_t *next;
};

/** Create new string list from a single string. */
stringlist_t* stringlist_new( const char* str );

/** Return last item in string list. */
stringlist_t* stringlist_back( stringlist_t* sl );

/** Push a new string to the front of a list.
 * @return Pointer to new front of list. */
stringlist_t* stringlist_push_front( stringlist_t* sl, const char* str );

/** Push a new string to the end of a list.
 * @return Pointer to the front of list. */
stringlist_t* stringlist_push_back( stringlist_t* sl, const char* str );

/** Return the length of the list, i.e. the number of strings in the list. */
unsigned stringlist_len( const stringlist_t* sl );

/** Free a strig list. */
void stringlist_free( stringlist_t* sl );

/** Trim a string on the right end (removes blanks). */
char *strtrim_right( char *p );

/** Trim a string on the right end (removes blanks). */
char *strtrim_right_e( char *beg, char *end );

/** Decode url encoded strings, destination and source can be the same string.
 * Url decoding functions taken from stringencoders sources by Nick Galbreath
 * https://code.google.com/p/stringencoders/ */
size_t url_decode( char *dest, const char *src );

/** Decode url encoded strings up to a given length, 
 * destination and source can be the same string.
 * Url decoding functions taken from stringencoders sources by Nick Galbreath
 * https://code.google.com/p/stringencoders/ */
size_t url_decode_l( char *dest, const char *src, const size_t slen );

/** @} */

#endif /* STR_UTILS_H_ */
