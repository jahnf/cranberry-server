/* cranberry-server. A small C web server application with lua scripting,
 * session and sqlite support. https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */

/** @file version.c */

#include "version.h"

#ifdef GITVERSION
    #include "version/version_string.h"
#endif

#if defined(VERSION_STRING)
    static const char version_string[] = VERSION_STRING;
#else
    static const char version_string[] = "?";
#endif

#if BUILD_TYPE_ENUM
    static const int build_type   = BUILD_TYPE_ENUM;
#else
    static const int build_type   = BUILD_TYPE_UNKNOWN;
#endif

const char* get_version_string()
{
    return version_string;
}

int get_build_type()
{
    return build_type;
}
