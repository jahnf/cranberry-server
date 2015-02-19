#ifndef KV_ITER_H__
#define KV_ITER_H__

/** @defgroup kviter Key/Value String Iterator
 * Parse C-strings for key/value pairs - for example in 
 * HTTP GET-requests. Uses no heap, makes no copy and does not 
 * make  modification on the input data. All credit goes to Nick Galbreath
 * and his stringencoders project (https://code.google.com/p/stringencoders/).
 * With minor changes the code is an exact copy for the modp_qsiter.
 * 
 * Example usage:
 *
 * @code{.c}
 * kviter_t kvi;
 * const char* qs = "foo=bar&ding=bar";
 * kviter_reset( &kvi, '&', '=', qs, strlen(qs) );
 * 
 * // Iterate over all key/value's in qs
 * while (kviter_next(&kvi)) 
 * {
 *    // we only get start and length of key and value 
 *    // - the rest is up to the user
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
 * @endcode
 *
 * @{
 * @file kv_iter.h Header file. 
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
/** Key-Value iterator struct. */
typedef struct {
    char key_sep;   /*!< Key separator character. */
    char val_sep;   /*!< Value separator character. */

    const char* s;
    size_t pos;
    size_t len;

    const char* key;
    size_t keylen;

    const char* val;
    size_t vallen;
} kviter_t;


/**
 * Reset an iterator to an initial start (constructor).
 *
 * This does not modify the original string, nor makes a copy.
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

/**
 * Get next key/value pair in query string
 *
 * \param[out] kvi data struct
 * \param[in] ignore_leading_char Ignore character c in at the beginning of key values.
 * \param[in] c Character to ignore if ignore_leading_char is != 0.
 * \return true if found a key value pair, false if no more data
 */
int kviter_next_i( kviter_t* kvi, int ignore_leading_char, char c );

#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* KV_ITER_H__ */
