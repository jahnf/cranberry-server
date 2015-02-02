/* strawberry-ini dictionary (iniDictionary)

The strawberry-ini dictionary is distributed under the MIT License:

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

/* Changelog / strawberry-ini dictionary
 *
 * Version 1.0.0
 *  - first version released to the public
 *
 */

/**
 * @file ini_dictionary.h
 * @brief iniDictionary - holds key,value pairs and sections of configurations
 */

#ifndef __STRAWBERRY_INI_DICTIONARY_H__
#define __STRAWBERRY_INI_DICTIONARY_H__

#ifdef __cplusplus
extern "C" {
#endif

/** The version number of the strawberry-ini dictionary. */
#define VERSION_STRAWBERRY_INI_DICTIONARY "1.0.0"

/** Nonzero to compile with functionality to write a dictionary
 * (to stdout or a file) - the default is 0. */
#ifndef INI_DICTIONARY_DUMP
#define INI_DICTIONARY_DUMP 0
#endif

/** An ini item contains a key/value pair and a pointer to the next item. */
typedef struct ini_item ini_item;
/** An ini item contains a key/value pair and a pointer to the next item. */
struct ini_item {
    char *key;          /*!< key name */
    char *value;        /*!< value */
    ini_item *next;      /*!< pointer to next item */
};

/** An ini section containing the name and pointer to the next section
 * and the first and last item within the section. */
typedef struct ini_section ini_section;
/** An ini section containing the name and pointer to the next section
 * and the first and last item within the section. */
struct ini_section{
    char *name;          /*!< section name */
    ini_item *first_item; /*!< pointer to the first item in the section */
    ini_item *last_item;  /*!< pointer to the last item in the section */
    ini_section *next;    /*!< pointer to next section */
};

/** Contains the loaded structure of an INI file. */
typedef struct {
    ini_section *first_section; /*!< pointer to first section in the dictionary */
    ini_section *last_section;  /*!< pointer to last section in the dictionary */
} ini_dictionary;


/**
 * @brief get the string value for a section and a key.
 * @param [in] dict		 pointer to dictionary
 * @param [in] section	 section name
 * @param [in] key		 key name
 * @param [in] def		 default value
 * @return Pointer to a character string.
 *   If the section and the key are found inside the dictionary, the
 *   pointer to the corresponding value string is returned. Otherwise
 *   the default value is returned.
 */
char * ini_dictionary_getstring( ini_dictionary *dict, const char *section, const char * key, const char *def );

/**
 * @brief get the integer value for a section and key
 * @param [in] dict			pointer to dictionary
 * @param [in] section		section name
 * @param [in] key			key name
 * @param [in] notfound		default value
 * @return integer value of the corresponding value in the given
 *   section and key, the default value otherwise. If a key has
 *   an empty string as value, it is considered as not set.
 */
int ini_dictionary_getint( ini_dictionary * dict, const char *section, const char * key, const int notfound );

/**
 * @brief get the double value for a section and key
 * @param [in] dict			pointer to dictionary
 * @param [in] section		section name
 * @param [in] key			key name
 * @param [in] notfound		default value
 * @return double value of the corresponding value in the given section and key,
 *   the default value otherwise. If a key has an empty string as value,
 *   it is considered as not set.
 */
double ini_dictionary_getdouble( ini_dictionary * dict, const char *section, const char * key, const double notfound );

/**
 * @brief get the boolean value for a section and key.
 *   Every value that starts with '1','T','t','Y','y' is treated as boolean true,
 *   all other as false.
 * @param [in] dict			pointer to dictionary
 * @param [in] section		section name
 * @param [in] key			key name
 * @param [in] notfound		default value
 * @return boolean (int) value of the corresponding value in the given
 *   section and key, the default value otherwise. If a key has an empty
 *   string as value, it is considered as not set.
 *
 */
int ini_dictionary_getboolean( ini_dictionary * dict, const char *section, const char * key, int notfound );


/**
 * @brief Allocate new dictionary.
 * @return	Pointer to allocated dictionary or NULL on error.
 */
ini_dictionary * ini_dictionary_new( void );

/**
 * @brief Free a dictionary.
 *   The functions frees the memory of a dictionary and it's content.
 *   After the call of this function dict points to unallocated memory.
 * @param [in] dict	pointer to dictionary
 */
void ini_dictionary_free( ini_dictionary *dict );

/**
 * @brief Get a section from a dictionary with a given name.
 * @param [in] dict	pointer to dictionary
 * @param [in] name	section name
 * @return Pointer to an ini_section, if the section was found, NULL otherwise.
 */
ini_section * ini_dictionary_get_section( ini_dictionary *dict, const char *name );

/**
 * @brief Add a section to a dictionary.
 * @param [in] dict	pointer to dictionary
 * @param [in] name	section name
 * @return Pointer to an ini_section, or NULL on allocation error.
 *   If the section with the given name does not exist, the section is created and
 *   the pointer to the new section is returned. If the section already exists,
 *   the pointer to the existing section is returned.
 */
ini_section * ini_dictionary_add_section( ini_dictionary *dict, const char *name );

/**
 * @brief Remove a section from a dictionary.
 *   This functions removes a section from a dictionary, including all
 *   keys the section might contain.
 * @param [in] dict	pointer to dictionary
 * @param [in] name section name
 * @return 1 if section was found (and removed), 0 otherwise.
 */
int ini_dictionary_del_section( ini_dictionary *dict, const char *name );

/**
 * @brief Get the numbers of sections in a dictionary.
 * @param [in] dict dictionary pointer
 * @return The numbers of sections.
 */
unsigned int ini_dictionary_section_count( const ini_dictionary *dict );

/**
 * @brief Get a dictionary entry from a section.
 * @param [in] section	pointer to section
 * @param [in] key 		name of the key
 * @return Pointer to dictionary_item if found, NULL otherwise.
 */
ini_item * ini_section_get_item( ini_section *section, const char *key );

/**
 * @brief Remove an item with the given key from a section.
 *   This functions removes an item from a section
 *   keys the section might contain.
 * @param [in] section pointer to a section
 * @param [in] key
 * @return 1 if the item was found (and removed), 0 otherwise.
 */
int ini_section_del_item( ini_section *section, const char *key );

/**
 * @brief Get a dictionary entry's value from a section.
 * @param [in] section	pointer to section
 * @param [in] key		key name
 * @return Pointer to string value if found, NULL otherwise.
 */
char * ini_section_get_value( ini_section *section, const char *key );

/**
 * @brief Add a dictionary entry to a section.
 *   If the dictionary item to key already exists, the value of this item will
 *   be overwritten and the pointer to the existing item is returned.
 *   If the dictionary item was not found, a new item is created and the
 *   pointer is returned.
 * @param [in] section
 * @param [in] key
 * @param [in] value
 * @return Pointer to new or existing dictionary item. NULL on allocation error.
 */
ini_item * ini_section_add_item( ini_section *section, const char *key, const char *value );

/**
 * @brief Get the number of dictionary items in a section.
 * @param [in] section
 * @return Number of items.
 */
unsigned int ini_section_item_count( const ini_section *section );

#if INI_DICTIONARY_DUMP
#include <stdio.h>
    void ini_dictionary_tostdout( ini_dictionary *dict );
    int ini_dictionary_tofile( ini_dictionary *dict, FILE* fp );
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STRAWBERRY_INI_DICTIONARY_H__ */
