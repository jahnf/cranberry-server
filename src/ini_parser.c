/* strawberry-ini parser (iniParser)

The strawberry-ini parser is a modified and extended version of inih.
 inih Copyright (c) 2009, Brush Technology
 distributed under the New BSD license
 (http://code.google.com/p/inih/source/browse/trunk/LICENSE.txt)
 for details see the inih project home page: http://code.google.com/p/inih/

The strawberry-ini parser is distributed under the BSD 2-Clause License:

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

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "ini_parser.h"

#if !INI_USE_STACK
#include <stdlib.h>
#endif

#ifndef __cplusplus
    #ifdef _MSC_VER
    /* inline keyword is not available in Microsoft C Compiler */
    #define inline __inline
    #endif
#endif

/* Strip whitespace chars off end of given string, in place. Return new end. */
inline static char *rstrip_e( char *beg, char *end ) {

    while( end > beg && isspace((int)*--end)) {
        *end = '\0';
    }
    return beg;
}

/* Return pointer to first non-whitespace char in given string. */
inline static char* lskip(const char* s)
{
    for ( ;*s && isspace((int)*s); ++s );
    return (char*)s;
}

#if INI_NAME_AND_SECTION_LOWER_CASE
    /* Version of strncpy that ensures dest (size bytes) is null-terminated
     * while converting characters to lower case */
    inline static void strncpy0_tolower( char *dest, const char* src, size_t size ) {
        for( ; *src && size; --size )
            *(dest++) = (char)tolower((int)*src++);
        if( size ) {
            *dest = '\0';
        } else {
            *(--dest) = '\0';
        }
    }
    #define strncpy0(a,b,c) strncpy0_tolower(a,b,c)
    inline static void str_tolower( char *s ) {
      for( ; *s; ++s )
          *s = (char)tolower((int)*s);
    }
#else
    /* Version of strncpy that ensures dest (size bytes) is null-terminated. */
    inline static char* strncpy0(char* dest, const char* src, size_t size)
    {
        for( ; *src && size; --size )
            *(dest++) = (char)(*src++);
        if( size ) {
            *dest = '\0';
        } else {
            *(--dest) = '\0';
        }
    }
#endif

/* if err_in is an error code that represents a severe error
 * err_out is assigned that error code and line_out is assigned
 * the curr_line parameter */
inline static int is_severe_error( const int err_in, int *err_out,
                                   const unsigned curr_line, unsigned *line_out ) {
    switch( err_in ) {
    case INI_ERR_NULLPTR:
    case INI_ERR_FREAD:
    case INI_ERR_FOPEN:
    case INI_ERR_MALLOC:
        *err_out = err_in;
        if( line_out ) *line_out = curr_line;
        return 1;
    default:
        return 0;
    }
    return 0;
}

/* See documentation in header file. */
int ini_parse_file(FILE* file,
                   int (*handler)(void*, const char*, const char*,
                                  const char*),
                   void* user, unsigned* line_err)
{
    /* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
    char line[INI_MAX_LINE];
    char section[INI_MAX_SECTION] = "";
    char *start = line;
#else
    char* line;
    char* section;
    char* start;
#endif

    char *end, *name, *value;
    int error = 0;
    int last = 0;

    unsigned lineno = 0;

#if !INI_USE_STACK
    /* stack usage is not defined, we need to allocate memory */
    line = (char*)malloc(INI_MAX_LINE);
    if( line = NULL ) {
        return INI_ERR_MALLOC;
    }

    section = (char*)malloc(INI_MAX_SECTION);
    if( section == NULL ) {
        free( line );
        return INI_ERR_MALLOC;
    }
    start = line;
#endif

    /* Scan through file line by line */
    while( fgets(line+last, INI_MAX_LINE-last, file) != NULL ) {

        ++lineno;

        /* end points to trailing 0 char*/
        end = strchr( line+last, '\0' );

        if( end >= &line[INI_MAX_LINE-1] && *(end-1) != '\n' ) {
            /* line too long (does not fit in line buffer[INI_MAX_LINE]) */
            #if INI_OVERLONG_LINE_IS_ERROR
                if( !error ) {
                    error = INI_ERR_LINE_TOO_LONG;
                    if( line_err ) *line_err = lineno;
                }
                #if INI_STOP_ON_ERROR
                    break; /* break while */
                #endif
            #endif

            /* line is too long, but not considered an error
             * we just read the file stream until \n or EOF
             * this skips rest of the line */
            do {
                last = getc( file );
            } while( last != EOF  && last != '\n' );
        }

#if INI_SKIP_BOM
        /* if there is a utf8 byte order mark - skip it */
        if( lineno == 1 && (unsigned char)start[0] == 0xEF &&
                           (unsigned char)start[1] == 0xBB &&
                           (unsigned char)start[2] == 0xBF ) {
            start += 3;
        }
#endif

        /* Get rid of white spaces at end of line */
        for( ; (end > start) && (isspace((int)*--end)); )
            *end = '\0';
        /* end now points to last character before trailing 0 if end != start */

        /* trim spaces at the beginning of the line */
        for( ; isspace((int)*start); ++start );

        if( *start == ';' || *start == '#' || *start == '\0' ) {
            /* Skip lines with '#' and ';' at start of line as comments */
            /* also skip empty lines */
        }
#if INI_ALLOW_MULTILINE
        /* Detect multi-line - line ends with '\' */
        else if( *end=='\\' ) {
            /* remember position and go on reading from stream */
            last=end-line;
            continue;
        }
#endif
        else if( *start == '[' ) {
            /* A "[section]" line */
            if( *end == ']' ) {
                *end = '\0';
                strncpy0(section, start + 1, INI_MAX_SECTION);
#if INI_SECTION_CALLBACK
                /* call handler with section arguments */
                if( ( last = handler(user, section, NULL, NULL) ) != INI_OKAY) {
                    if( is_severe_error( last, &error, lineno, line_err) ) {
                        break; /* always stop on severe errors */
                    } else if( !error ) {
                        error = last;
                        if( line_err ) *line_err = lineno;
                        #if INI_STOP_ON_ERROR
                            break;
                        #endif
                    }
                }
#endif
            }
            else if( !error ) {
                /* No ']' found on section line */
                error = INI_ERR_SYNTAX;
                if( line_err ) *line_err = lineno;
                #if INI_STOP_ON_ERROR
                    break;
                #endif
            }
        }
        else if( *start ) {
#if INI_SUPPORT_QUOTES
            char *val_end = end;
#endif
            /* Not a comment, not a section,
             * should be a name[=:]value pair
             *
             * Find first = or : whatever comes first...
             * */
            if( (last = strcspn( start, "=:" ))
                    && (value = end+1)
                    && (end = start+last) < value ) {

                *end = '\0';
                rstrip_e( start, end );
                name = start;
#if INI_NAME_AND_SECTION_LOWER_CASE
                str_tolower( name );
#endif
                value = lskip( end + 1 );
#if INI_SUPPORT_QUOTES
                if( val_end >= value ) {
                    if ( ((value)[0] == '"'  && *val_end == '"' ) ||
                         ((value)[0] == '\'' && *val_end == '\'') ) {
                        *val_end = '\0';
                        ++value;
                    }
                }
#endif

                /* Valid name[=:]value pair found, call handler */
                if( ( last = handler(user, section, name, value) ) != INI_OKAY ) {
                    if( is_severe_error( last, &error, lineno, line_err) ) {
                        break; /* always stop on severe errors */
                    } else if( !error ) {
                        error = last;
                        if( line_err ) *line_err = lineno;
                        #if INI_STOP_ON_ERROR
                            break;
                        #endif
                    }
                }

            } else if( !error )  {
                /* No '=' or ':' found on name[=:]value line */
                error = INI_ERR_SYNTAX;
                if( line_err ) *line_err = lineno;
                #if INI_STOP_ON_ERROR
                    break;
                #endif
            }
        }

        last = 0;
        start = line;
    } /* end while */

#if !INI_USE_STACK
    free(line);
    free(section);
#endif

    if( ferror(file) && !error )
          error = INI_ERR_FREAD;

    return error;
}

/* See documentation in header file. */
int ini_parse(const char* filename,
              int (*handler)(void*, const char*, const char*, const char*),
              void* user, unsigned* line_err )
{
    FILE* file;
    int error;

    if( (file=fopen(filename, "r"))==NULL ) {
        return INI_ERR_FOPEN ;
    }

    error = ini_parse_file(file, handler, user, line_err );
    fclose(file);
    return error;
}

/* helper function - readable ini error code is returned
 * for err_code parameter */
const char* ini_errcode_tostr( const int err_code ) {
    #define CASE_X_STRX(x) case x: return #x
    switch( err_code ) {
    CASE_X_STRX(INI_OKAY);
    CASE_X_STRX(INI_ERR_FOPEN);
    CASE_X_STRX(INI_ERR_FREAD);
    CASE_X_STRX(INI_ERR_LINE_TOO_LONG);
    CASE_X_STRX(INI_ERR_MALLOC);
    CASE_X_STRX(INI_ERR_NULLPTR);
    CASE_X_STRX(INI_ERR_SYNTAX);
    CASE_X_STRX(INI_ERR_OTHER);
    }
    return "";
}
