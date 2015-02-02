/* strawberry-ini reader (iniReader) - read configuration into a dictionary

The strawberry-ini reader is distributed under the MIT License:

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

#include "ini_reader.h"

typedef struct {
    ini_section *cur_section;
    ini_dictionary *dict;
} ini_reader_userdata;

static int read_handler( void* user, const char* section,
                           const char* name, const char* value)
{
    ini_reader_userdata *ud = (ini_reader_userdata*) user;

#if INI_SECTION_CALLBACK
    /* section callback */
    if( name == NULL ) {
#endif
        if( !(ud->cur_section = ini_dictionary_add_section( ud->dict, section )) )
            return INI_ERR_MALLOC;
#if INI_SECTION_CALLBACK
        return INI_OKAY;
    }

    if( !ud->cur_section &&
            !(ud->cur_section = ini_dictionary_add_section( ud->dict, "" )) )
        return INI_ERR_MALLOC;
#endif

    /* section, name, value */
    if( !(ini_section_add_item( ud->cur_section, name, value )) )
        return INI_ERR_MALLOC;

    return INI_OKAY;
}

int ini_readfile( const char *filename, ini_dictionary **__dict, unsigned *line_err )
{
    int ret = INI_OKAY;
    ini_reader_userdata userdata = {NULL, NULL};

    if( __dict == NULL )
        return INI_ERR_NULLPTR;

    if( (*__dict = ini_dictionary_new()) == NULL ) {
        return INI_ERR_MALLOC ;
    }

    userdata.dict = *__dict;
    ret = ini_parse( filename, &read_handler, &userdata, line_err );

    if( ret != INI_OKAY ) {
        ini_dictionary_free( *__dict );
        *__dict = NULL ;
    }

    return ret;
}
