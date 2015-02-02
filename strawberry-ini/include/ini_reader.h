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

/* Changelog / strawberry-ini reader
 *
 * Version 1.0.0
 *  - first version released to the public
 *
 */

/**
 * @file ini_reader.h
 * @brief iniReader - read configuration into a dictionary, uses
 *   @link ini_parser.h iniParser @endlink and
 *   @link ini_dictionary.h iniDictionary @endlink to do that.
 */

#ifndef __STRAWBERRY_INI_READER_H__
#define __STRAWBERRY_INI_READER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ini_parser.h"
#include "ini_dictionary.h"

/** The version number of the strawberry-ini reader. */
#define VERSION_STRAWBERRY_INI_READER "1.0.0"

/**
 * @brief Read an INI file into a dictionary.
 * All sections and key values will be converted to lower case.
 * @param [in] filename INI filename.
 * @param [out] __dict A pointer to an ini_dictionary pointer.
 *   If the return value is #INI_OKAY, the dictionary pointer is set to a new
 *   allocated dictionary. If the return value is other than INI_OKAY no dictionary will
 *   be allocated on function return and the value of the dictionary pointer is
 *   unspecified.
 * @param [out] line_err If an error occurs and the line number is available and line_err != NULL,
 * line_err will be set to the line number where the error occurred.
 * @return Return code.
 *   #INI_OKAY if everything went okay, an INI_ERR_xxx value else.
 *
 */
int ini_readfile( const char *filename, ini_dictionary **__dict, unsigned *line_err );

#ifdef __cplusplus
}
#endif

#endif /* __STRAWBERRY_INI_READER_H__ */
