/* strawberry-ini parser (iniParser)

The strawberry-ini parser is a modified and extended version of inih.
 inih Copyright (c) 2009, Brush Technology
 distributed under the New BSD license
 (http://code.google.com/p/inih/source/browse/trunk/LICENSE.txt)
 for details see the inih project home page: http://code.google.com/p/inih/

The strawberry-ini parser is distributed under the MIT License:
 
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

/**
 * @file ini_parser.h
 * @brief iniParse - parse configuration files.
 *
 * @mainpage strawberry-ini API Documentation
 * strawberry-ini has consists of three parts of which most can be used seperately
 * if needed:
 * - @link ini_parser.h iniParser @endlink: Parse configuration files.
 *   The main functions are #ini_parse and #ini_parse_file.
 * - @link ini_dictionary.h iniDictionary @endlink:
 *   Functions and data structures to manage configuration settings.
 * - @link ini_reader.h iniReader @endlink: fills a dictionary, uses
 *   @link ini_parser.h iniParser @endlink and
 *   @link ini_dictionary.h iniDictionary @endlink to do that.
 *  
 * See also the @link readme README @endlink for an introduction.
 * 
 * Find the latest version on github: https://github.com/jahnf/strawberry-ini
 */

/* Changelog / strawberry-ini parser
 *
 * Version 1.0.0
 *  - first version released to the public
 *
 */

#ifndef __STRAWBERRY_INI_PARSER_H__
#define __STRAWBERRY_INI_PARSER_H__

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/** The version number of the strawberry-ini parser. */
#define VERSION_STRAWBERRY_INI_PARSER "1.0.0"

enum {
    INI_OKAY = 0,           /*!< everything went okay. */
    INI_ERR_NULLPTR,        /*!< a parameter was an unexpected NULL pointer. */
    INI_ERR_FOPEN,          /*!< error while opening the ini file. */
    INI_ERR_FREAD,          /*!< error while reading the ini file. */
    INI_ERR_MALLOC,         /*!< an allocation error occured */
    INI_ERR_LINE_TOO_LONG,  /*!< an input line from the INI file was longer than INI_MAX_LINE */
    INI_ERR_SYNTAX,         /*!< syntax error in INI file */
    INI_ERR_OTHER           /*!< other error */
};

/** helper function - readable ini error code is returned
 * for err_code parameter
 */
const char* ini_errcode_tostr( const int err_code );

/** Handler function callback signature. */
typedef int (*ini_handler) (void* user, const char* section,
                              const char* name, const char* value) ;

/** Parse given INI-style file. May have [section]s, name/value pairs
 * (white spaces are stripped), and comments starting with @c ';' (semicolon)
 * or @c '#'. The section name is empty ("") if a name/value pair is
 * parsed before any section heading.
 * @c name=value and @c name:value pairs are also supported.
 *
 * For each name/value pair parsed, call handler function with given
 * user pointer as well as section, name, and value (data only valid for
 * duration of handler call). Handler should return #INI_OKAY on success,
 * other @b INI_ERR_xxx values on error.
 *
 * If #INI_SUPPORT_QUOTES is not set to 0 values can be surrounded by
 * single or double quotes, i.e. if values need to have leading or
 * trailing white spaces.
 *
 * Returns #INI_OKAY on success, @b INI_ERR_xxx codes else.
 * @b line_err is set to the error line number if available.
 * The error always indicates the first occurring error. strawberry-ini
 * can be configured not to stop on error  with the define
 * #INI_STOP_ON_ERROR (but will always stop on #INI_ERR_NULLPTR,
 * #INI_ERR_FOPEN, #INI_ERR_FREAD and #INI_ERR_MALLOC)
 */
int ini_parse(const char* filename, ini_handler handler,
              void* user, unsigned* line_err);

/** Same as ini_parse(), but takes a FILE* argument instead of a filename.
 * This function does not close the file when finished --
 * it's the callers responsibility.
 */
int ini_parse_file(FILE* file, ini_handler handler,
                   void* user, unsigned* line_err);

/** Non-zero to allow multi-line value parsing, where the line has to
 * end with a backslash (\) so indicate the continuation in the next
 * line - if not otherwise defined the default is 1.
 * If multi-line parsing is allowed, the length of concatenated
 * lines still only can have INI_MAX_LINE characters .
 */
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

/** Non-zero for section callbacks - if not otherwise defined the default is 1.
 * The handler function will get called if a section is found,
 * parameters name and value are NULL in this case.
 */
#ifndef INI_SECTION_CALLBACK
#define INI_SECTION_CALLBACK 1
#endif

/** Non-zero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start
  * of the file - if not otherwise defined the default is 1.
  */
#ifndef INI_SKIP_BOM
#define INI_SKIP_BOM 1
#endif

/** Non-zero to use stack, zero to use heap (malloc/free) -
 * if not otherwise defined the default is 1.
 */
#ifndef INI_USE_STACK
#define INI_USE_STACK 1
#endif

/** Non-zero to support quotes (double and single quotes) -
 * if not otherwise defined the default is 1.
 */
#ifndef INI_SUPPORT_QUOTES
#define INI_SUPPORT_QUOTES 1
#endif

/** Maximum line length for a line in an INI file -
 * if not otherwise defined the default is 200.
 */
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif

/** Maximum length of a section name -
 * if not otherwise defined the default is 50. */
#ifndef INI_MAX_SECTION
#define INI_MAX_SECTION 50
#endif

/** Non-zero to immediately stop parsing on error -
 * if not otherwise defined the default is 1. */
#ifndef INI_STOP_ON_ERROR
#define INI_STOP_ON_ERROR 1
#endif

/** Non-zero to make section and name strings lower case - 
 * if not otherwise defined the default is 1.
 */
#ifndef INI_NAME_AND_SECTION_LOWER_CASE
#define INI_NAME_AND_SECTION_LOWER_CASE 1
#endif

/** Non-zero to treat overlong lines as an error -
 * if not otherwise defined the default is 0.
 * If over long lines are not treated as an error, lines are simply
 * cut off at INI_MAX_LINE characters.
 */
#ifndef INI_OVERLONG_LINE_IS_ERROR
#define INI_OVERLONG_LINE_IS_ERROR 0
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STRAWBERRY_INI_PARSER_H__ */
