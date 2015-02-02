#include <stdio.h>
#include <string.h>

#include <ini_reader.h>

/* A simple ini reader example
 *  - reads ini file into a dictionary
 *  - if successful we can then look for certain
 *    values in the dictionary or just iterate over it.
 */

int main ( int argc, char **argv )
{
    if( argc > 1) {
        ini_dictionary *dict = NULL;
        unsigned line_err;
        int err_code = ini_readfile( argv[1], &dict, &line_err );

        printf("input file: '%s'\n", argv[1]);
        printf("ini_readfile returned (%d) %s\n",
                    err_code, ini_errcode_tostr(err_code));

        /* give line number for certain error types */
        if( INI_ERR_SYNTAX == err_code || INI_ERR_LINE_TOO_LONG == err_code ) {
            printf( "error line number: %u\n", line_err );
        }
        if( INI_OKAY == err_code ) {
            /* everything went smooth */
            printf("ini section count: %u\n", ini_dictionary_section_count(dict));

            { /* iterate over all sections and print name and item count*/
                ini_section *section = dict->first_section;
                for( ; section != NULL; section = section->next ) {
                    printf("section [%s] : item_count = %u\n",
                           section->name, ini_section_item_count(section) );
                    /* loop through section items */
                    ini_item *item = section->first_item;
                    for( ; item != NULL; item = item->next ) {
                        printf("%s=%s\n", item->key, item->value );
                    }
                }
            }

            { /* look up defined values in the dictionary */
                /* the last parameter is the default value returned if no
                 * such section and key was found */
                const char* title = ini_dictionary_getstring( dict, "window", "title", "Default" );
                int active = ini_dictionary_getboolean( dict, "window", "active", 0 );
                printf( "\nWindowTitle = '%s'\n", title );
                printf( "WindowActive = %d\n", active );
            }

            /*  For more functions to interact with dictionaries,
             *  see the documentations in ini_dictionary.h */

            /* we need to make sure we free the
             * allocated dictionary memory */
            ini_dictionary_free( dict );
        }
    } else {
        printf("usage: example-01 filename\n");
    }
    return 0;
}
