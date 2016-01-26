/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
#ifndef STR_UTILS_H_
#define STR_UTILS_H_

#ifndef __cplusplus
#ifdef _MSC_VER
/* inline keyword is not available in Microsoft C Compiler */
#define inline __inline
#endif
#endif

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

/** djb2 string hash function. This algorithm (k=33) was first reported by dan
 * bernstein many years ago in comp.lang.c. Another version of this algorithm 
 * (now favored by bernstein) uses xor: `hash(i) = hash(i - 1) * 33 ^ str[i];` 
 * the magic of number 33 (why it works better than many other constants, 
 * prime or not) has never been adequately explained.
 * see also http://www.cse.yorku.ca/~oz/hash.html */
unsigned long strhash(const char* s);

/** Modifies a string to lower case. */
void str_tolower( char *s );

/** Reverse a character string. */
inline void strreverse(char* begin, char* end);

/** Trim a string on the right end (removes blanks). */
char *strtrim_right( char *p );

/** Trim a string on the right end (removes blanks).*/
char *strtrim_right_end( char *beg, char *end );

/** Returns a pointer to the first non-blank character in the given string. */
inline char *strtrim_left( char *p );

/** Combines the results of strtim_left and strtrim_right. */
inline char * strtrim( char * p );

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
