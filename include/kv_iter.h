#ifndef KV_ITER_H__
#define KV_ITER_H__

#ifdef __cplusplus
#define BEGIN_C extern "C" {
#define END_C }
#else
#define BEGIN_C
#define END_C
#endif

BEGIN_C

#include <string.h>
/**
 * Query string key value pair iterator.  Uses no heap, makes no copy and makes
 *  no modification of input.  Example usage:
 *
 * kviter_t kvi;
 * const char* qs = "foo=bar&ding=bar";
 * kviter_reset( &kvi, '&', '=', qs, strlen(qs) );
 * while (kviter_next(&kvi)) {
 *    // we only get start and length of key and value
 *    // the rest is up to the user
 *
 *    char* key = (char*) malloc(kvi.keylen + 1);
 *    key[kvi.keylen] = '\0';
 *    strncpy(key, kvi.key, kvi.keylen); 
 *    char* val = (char*) malloc(kvi.vallen + 1);
 *    val[kvi.vallen] = '\0';
 *    strcpy(val, kvi.val, kvi.vallen);
 *    printf("key = %s, value = %s\n", key, val);
 *    free(key);
 *    free(value);
 * }
 *
 *
 */
typedef struct {
	char key_sep;
	char val_sep;

    const char* s;
    size_t pos;
    size_t len;

    const char* key;
    size_t keylen;

    const char* val;
    size_t vallen;
} kviter_t;


/**
 * Reset a kviter to an initial start (constructor)
 *
 * This does not modifiy the original string, nor makes a copy.
 *
 * \param[out] kvi data struct used in iterator
 * \param[in] key_sep key separator character
 * \param[in] val_sep value separator character
 * \param[in] s input string (does not need to be 0-terminated)
 * \param[in] len input string length
 *
 */
void kviter_reset( kviter_t* kvi,
		const char key_sep, const char val_sep,
		const char* s, size_t len);

/**
 * Get next key/value pair in query string
 *
 * \param[out] kvi data struct
 * \return true if found a key value pair, false if no more data
 */
int kviter_next( kviter_t* kvi );
int kviter_next_i( kviter_t* kvi, int ignore_leading_char, char c );

END_C

#endif  /* KV_ITER_H__ */
