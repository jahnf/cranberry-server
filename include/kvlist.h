/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
#ifndef KVLIST_H_
#define KVLIST_H_

/** @defgroup kvlist Key/Value List
 * C-Style key/value item and list handling. 
 * Implemented with a simple single linked list approach.
 * @{
 * @file kvlist.h C-Style key/value item and list handling header. 
 */

/** Key/Value item. */
typedef struct kv_item_s {
   char * key;               /*!< Key string. */
   char * value;             /*!< Value string. */
   struct kv_item_s * next;  /*!< Pointer to next item. */
} kv_item;

/** Find first key-value item in a list with given key. 
 * @return Pointer to item or NULL if not found. */
kv_item * kvlist_find_key( const char *key, kv_item *kvlist );

/** Find first key-value item in a list with given key and return the value. 
 * @return Pointer to item value or NULL if not found. */
char * kvlist_get_value_from_key( const char *key, kv_item *kvlist );

/** Check if given list has an key-value item with given key. 
 * @return 1 if the key contains in the list, 0 otherwise. */
int kvlist_has_key( const char *key, kv_item *kvlist );

/** Free an entire key-value item list. */
void kvlist_free( kv_item *kvlist );

/** Create a new key-value item (This is a list containing only the new item). 
 * @return Pointer to the newly created item or NULL on failure. */
kv_item * kvlist_new_item( const char *key, const char *value );

/** Create a new key-value item and push in front of an existing item list.
 * @return Pointer to the new beginning of the list with the new item or NULL on failure. */
kv_item * kvlist_new_item_push_front( const char *key, const char *value, kv_item *exitem );

/** Create a new key-value item and push in front of an existing item list.
 * @return Pointer to the new beginning of the list with the new item or NULL on failure. */
kv_item * kvlist_new_item_push_front_ll( const char *key, const unsigned klen, const char *value,
                                            const unsigned vlen, kv_item *exitem );

/** Remove a given key-value item from a list. If the item was found the 
 * memory for the item itself is freed. 
 * @return 1 if the item was found and freed, 0 otherwise. */                                            
int kvlist_remove_item( kv_item **list, kv_item *item );

/** @} */

#endif /* KVLIST_H_ */
