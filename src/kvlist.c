/* C-Style key/value item and list handling. */

#include <string.h>
#include <stdlib.h>

#include "kvlist.h"

/** Find first key-value item in a list with given key */
kv_item * kvlist_find_key( const char *key, kv_item *kvlist ) {
    if( !key ) return 0;
    while( kvlist ) {
        if( kvlist->key && !strcmp(key, kvlist->key))
            return kvlist;
        kvlist = kvlist->next;
    }
    return NULL;
}

/** Find first key-value item in a list with given key and return the value */
char * kvlist_get_value_from_key( const char *key, kv_item *kvlist ) {
    kv_item *kvitem = kvlist_find_key( key, kvlist );
    if( !kvitem ) return NULL;
    return kvitem->value;
}

/** Check if given list has an key-value item with given key */
int kvlist_has_key( const char *key, kv_item *kvlist ) {
    if( kvlist_find_key( key, kvlist ) ) return 1;
    return 0;
}

/** free an entire key-value item list */
void kvlist_free( kv_item *kvlist ) {
    kv_item *ltemp = 0;
    while( kvlist ) {
        ltemp = kvlist;
        kvlist = kvlist->next;
        free( ltemp->key );
        free( ltemp->value );
        free( ltemp );
    }
}

int kvlist_remove_item( kv_item **list, kv_item *item ) {
    if( list && *list ) {
        kv_item *l_iter = *list, *l_prev = NULL;
        while( l_iter ) {
            if( item == l_iter ) {
                if( l_prev ) l_prev->next = l_iter->next;
                else *list = l_iter->next;
                free( l_iter->key );
                free( l_iter->value );
                free( l_iter );
                return 1;
            }
            l_prev =  l_iter;
            l_iter = l_iter->next;
        }
    }
    return 0;
}


/** creates a new key-value item (this is also a list containing only the new item) */
kv_item * kvlist_new_item( const char *key, const char *value ) {
    return kvlist_new_item_push_front(key, value, 0);
}

/** creates a new key-value item and in front of an existing item */
kv_item * kvlist_new_item_push_front( const char *key, const char *value, kv_item *exitem ) {
    kv_item *new_item = malloc(sizeof(kv_item));
    if( new_item != NULL ) {
        if( key != NULL	&& (NULL != (new_item->key = malloc(strlen(key)+1))) ) {
            strcpy(new_item->key, key);
            if( value != NULL ) {
                if( NULL != (new_item->value = malloc(strlen(value)+1)) )
                    strcpy(new_item->value, value);
            } else {
                new_item->value = NULL;
            }
        } else {
            new_item->key = NULL;
            new_item->value = NULL;
        }
        new_item->next = exitem;
    }
    return new_item;
}


/** creates a new key-value item and in front of an existing item */
kv_item * kvlist_new_item_push_front_ll( const char *key, const unsigned klen, const char *value,
                                            const unsigned vlen, kv_item *exitem ) {
    kv_item *new_item = malloc(sizeof(kv_item));
    if( new_item != NULL ) {
        if( key != NULL && (NULL != (new_item->key = malloc( klen+1 ))) ) {
            strcpy(new_item->key, key);
            if( value != NULL ) {
                if( NULL != (new_item->value = malloc(vlen+1)) )
                    strcpy(new_item->value, value);
            } else {
                new_item->value = NULL;
            }
        } else {
            new_item->key = NULL;
            new_item->value = NULL;
        }
        new_item->next = exitem;
    }
    return new_item;
}
