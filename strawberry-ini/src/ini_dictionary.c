/* straberry-ini dictionary (iniDictionary)

The straberry-ini dictionary is distributed under the MIT License:

The MIT License (MIT)

Copyright (c) 2013 Jahn Fuchs

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "ini_dictionary.h"

#ifndef __cplusplus
	#ifdef _MSC_VER
	/* inline keyword is not available in Microsoft C Compiler */
	#define inline __inline
	#endif
#endif

/* Defines ------------------------------------------------------------*/
static const char _ini_invalid_key_[] = "";
#define INI_INVALID_KEY     (_ini_invalid_key_)

inline static void ini_item_free( ini_item *item ) {
    if( item != NULL ) {
        free( item->key );
        free( item->value );
        free( item );
    }
}

static void ini_section_free( ini_section *section ) {
    if( section != NULL ) {
        ini_item *pdi_tmp;
        while( section->first_item ) {
            pdi_tmp = section->first_item;
            section->first_item = section->first_item->next;
            ini_item_free( pdi_tmp );
        }
        free( section->name );
        free( section );
    }
}

char * ini_dictionary_getstring( ini_dictionary *dict, const char *sec_name,
                                 const char * key, const char *def ) {
    ini_section *section;
    char *value = NULL;

    if( dict == NULL || key == NULL )
        return (char*) def;

    if( (section = ini_dictionary_get_section( dict, sec_name )) )
        value = ini_section_get_value( section, key );

    /* only return the found value if it's not an empty string. */
    if( value && value[0] )
        return value;
    return (char*) def;
}

int ini_dictionary_getint( ini_dictionary * dict, const char *sec_name,
                           const char * key, const int notfound ) {
    const char *str = ini_dictionary_getstring( dict, sec_name, key, INI_INVALID_KEY );
    if( str == INI_INVALID_KEY )
        return notfound;
    return (int) strtol( str, NULL, 0 );
}

double ini_dictionary_getdouble( ini_dictionary * dict, const char *sec_name,
                                 const char * key, const double notfound ) {
    const char *str = ini_dictionary_getstring( dict, sec_name, key, INI_INVALID_KEY );
    if( str == INI_INVALID_KEY )
        return notfound;
    return atof( str );
}

int ini_dictionary_getboolean( ini_dictionary * dict, const char *sec_name,
                               const char * key, int notfound ) {
    const char *c = ini_dictionary_getstring( dict, sec_name, key, INI_INVALID_KEY );
    if( c == INI_INVALID_KEY )
        return notfound;

    switch( c[0] ) {
    case '1':  case '2':  case '3':
    case '4':  case '5':  case '6':
    case '7':  case '8':  case '9':
    case 'T':  case 'Y':
    case 't':  case 'y':
        return 1;
    default:
        break;
    }
    return 0;
}

ini_dictionary * ini_dictionary_new( void ) {

    ini_dictionary *newdict;

    if( NULL == (newdict = malloc( sizeof(ini_dictionary) )) )
        return NULL;

    newdict->first_section = NULL;
    newdict->last_section = NULL;
    return newdict;
}

void ini_dictionary_free( ini_dictionary *dict ) {
    if( dict != NULL ) {
        ini_section *sec_tmp;
        while( dict->first_section ) {
            sec_tmp = dict->first_section;
            dict->first_section = dict->first_section->next;
            ini_section_free( sec_tmp );
        }
        free( dict );
    }
}

unsigned int ini_dictionary_section_count( const ini_dictionary *dict ) {
    unsigned int c = 0;
    if( dict != NULL ) {
        ini_section *section;
        for( section = dict->first_section; section; ++c )
            section = section->next;
    }
    return c;
}

unsigned int ini_section_item_count( const ini_section *section ) {
    unsigned int c = 0;
    if( section != NULL ) {
        ini_item *dict_item;
        for( dict_item = section->first_item; dict_item; ++c )
            dict_item = dict_item->next;
    }
    return c;
}

static ini_section * _dictionary_get_section( const ini_dictionary *dict,
                                              const char *name, ini_section **prev ) {
    if( dict != NULL ) {
        ini_section *section = dict->first_section;
        *prev = NULL;
        for( ; section; section = section->next ) {
            if( strcmp( name, section->name ) == 0 ) {
                return section;
            }
            *prev = section;
        }
    }
    return NULL;
}

ini_section * ini_dictionary_get_section( ini_dictionary *dict, const char *name ) {
    static ini_section *tmp_section;
    return _dictionary_get_section( dict, name, &tmp_section );
}

static ini_item * _section_get_item( const ini_section *section,
                                     const char *key, ini_item **prev ) {
    if( section != NULL ) {
        ini_item *item = section->first_item;
        *prev = NULL;
        for( ; item; item = item->next ) {
            if( strcmp( key, item->key ) == 0 ) {
                return item;
            }
            *prev = item;
        }
    }
    return NULL;
}

ini_item *ini_section_get_item( ini_section *section, const char *key ) {
    static ini_item *tmp_item;
    return _section_get_item( section, key, &tmp_item );
}

int ini_dictionary_del_section( ini_dictionary *dict, const char *name ) {
    ini_section *prev_section;
    ini_section *section = _dictionary_get_section( dict, name, &prev_section );
    if( section != NULL ) {
        /* remove section from dictionary list */
        if( prev_section )
            prev_section->next = section->next;
        else
            dict->first_section = section->next;

        if( section == dict->last_section )
            dict->last_section = prev_section;

        ini_section_free( section );
        return 1;
    }
    return 0;
}

int ini_section_del_item( ini_section *section, const char *key ) {
    ini_item *prev_item;
    ini_item *item = _section_get_item( section, key, &prev_item );
    if( item != NULL ) {
        /* remove item from sections item list */
        if( prev_item )
            prev_item->next = item->next;
        else
            section->first_item = item->next;

        if( item == section->last_item )
            section->last_item = prev_item;

        ini_item_free( item );
        return 1;
    }
    return 0;
}


ini_section *ini_dictionary_add_section( ini_dictionary *dict, const char *name ) {
    if( dict != NULL ) {
        /* check if item already exists */
        ini_section *section = ini_dictionary_get_section( dict, name );

        if( section == NULL ) {
            /* key value not found, create dict_item with key and value */
            if( NULL == (section = malloc( sizeof(ini_section) )) )
                return NULL;

            if( NULL == (section->name = malloc( strlen( name ) + 1 )) ) {
                free( section );
                return NULL;
            }
            strcpy( section->name, name );
            section->next = NULL;
            section->first_item = section->last_item = NULL;

            if( section->name[0] == '\0' ) {
                /* make sure section with an empty name always is first section */
                section->next = dict->first_section;
                if( dict->last_section == NULL)
                    dict->last_section = section;
                dict->first_section = section;
            } else {
                if( dict->last_section )
                    dict->last_section->next = section;
                else
                    dict->first_section = section;
                dict->last_section = section;
            }
        }
        return section;
    }
    return NULL;
}

char * ini_section_get_value( ini_section *section, const char *key ) {
    ini_item *dict_item = ini_section_get_item( section, key );
    if( dict_item != NULL )
        return dict_item->value;
    return NULL;
}

ini_item *ini_section_add_item( ini_section *section, const char *key, const char *value ) {

    if( section != NULL ) {
        /* check if item already exists */
        ini_item *dict_item = ini_section_get_item( section, key );

        if( dict_item == NULL ) {
            /* key value not found, create dict_item with key and value */
            if( NULL == (dict_item = malloc( sizeof(ini_item) )) )
                return NULL;

            if( NULL == (dict_item->key = malloc( strlen( key ) + 1 )) ) {
                free( dict_item );
                return NULL;
            }
            strcpy( dict_item->key, key );
            dict_item->value = NULL;
            dict_item->next = NULL;

            if( section->last_item )
                section->last_item->next = dict_item;
            else
                section->first_item = dict_item;
            section->last_item = dict_item;
        }

        free( dict_item->value );
        if( NULL == (dict_item->value = malloc( strlen( value ) + 1 )) )
            return NULL;
        strcpy( dict_item->value, value );
        return dict_item;
    }
    return NULL;
}

#if INI_DICTIONARY_DUMP
/* print ini dictionary to stdout */
void ini_dictionary_tostdout( ini_dictionary *dict ) {
    if( dict != NULL ) {
        ini_item *dict_item;
        ini_section *section = dict->first_section;
        unsigned sec_count = 0;
        while( section ) {
            if( section->name ) {
                if( !(sec_count++ == 0 && section->name[0] == '\0') )
                    printf( "[%s]\n", section->name );
                dict_item = section->first_item;
                while( dict_item ) {
                    if( dict_item->key ) {
                        printf( "%s='%s'\n", dict_item->key,
                                 ((dict_item->value) ? dict_item->value : "") );
                    }
                    dict_item = dict_item->next;
                }
            }
            section = section->next;
        }
    }
}
/* ini_dictionary_tofile */
int ini_dictionary_tofile( ini_dictionary *dict, FILE* fp ) {
    if( dict != NULL ) {
        ini_item *dict_item;
        ini_section *section = dict->first_section;
        unsigned int sec_count = 0;
        while( section ) {
            if( section->name ) {
                if( !(sec_count++ == 0 && section->name[0] == '\0') ) {
                    if( 1 != fwrite("[", 1, 1, fp ) ) return -1;
                    if( 1 != fwrite(section->name, strlen(section->name), 1, fp ) ) return -1;
                    if( 1 != fwrite("]\n", 2, 1, fp ) ) return -1;
                }
                dict_item = section->first_item;
                while( dict_item ) {
                    if( dict_item->key ) {
                        if( 1 != fwrite(dict_item->key, strlen(dict_item->key), 1, fp ) ) return -1;
                        if( 1 != fwrite("='", 2, 1, fp ) ) return -1;
                        if( dict_item->value != NULL && dict_item->value[0] != '\0' )
                            if( 1 != fwrite(dict_item->value, strlen(dict_item->value), 1, fp ) ) return -1;
                        if( 1 != fwrite("'\n", 2, 1, fp ) ) return -1;
                    }
                    dict_item = dict_item->next;
                }
            }
            section = section->next;
        }
    }
    return 0;
}
#endif
