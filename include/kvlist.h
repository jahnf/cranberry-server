/* C-Style key/value item and list handling. */

#ifndef KVLIST_H_
#define KVLIST_H_

typedef struct kv_item_s {
   char * key;
   char * value;
   struct kv_item_s * next;
} kv_item;

/** Find first key-value item in a list with given key */
kv_item * kvlist_find_key( const char *key, kv_item *kvlist );

/** Find first key-value item in a list with given key and return the value */
char * kvlist_get_value_from_key( const char *key, kv_item *kvlist );

/** Check if given list has an key-value item with given key */
int kvlist_has_key( const char *key, kv_item *kvlist );

/** free an entire key-value item list */
void kvlist_free( kv_item *kvlist );

/** creates a new key-value item (this is also a list containing only the new item) */
kv_item * kvlist_new_item( const char *key, const char *value );

/** creates a new key-value item and in front of an existing item */
kv_item * kvlist_new_item_push_front( const char *key, const char *value, kv_item *exitem );
kv_item * kvlist_new_item_push_front_ll( const char *key, const unsigned klen, const char *value,
                                            const unsigned vlen, kv_item *exitem );

int kvlist_remove_item( kv_item **list, kv_item *item );

#endif /* KVLIST_H_ */
