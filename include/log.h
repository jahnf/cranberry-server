/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
#ifndef LOG_H_
#define LOG_H_

/** @defgroup logging Logging
 * Logging module for logging to console and/or a file with support
 * for different logging levels.
 *
 * This module can be disabled by defining NO_LOGGING with a value other than 0
 * for log.c, log.h and all files that use log.h
 * The logging macros then are empty and no overhead is produced.
 *
 * How to use logging:
 * - `#include "log.h"`
 * - Set the logging module name by using one of the following macros
 *   `SETLOGMODULENAME(name)` or `SETDEFAULTLOGMODULENAME()`.
 *   - __Example 1__: Sets the logging module name to `"myModule"`:
 *    `SETLOGFILENAME("myModule");`
 *   - __Example 2__: If `__FILE__` is `/home/dev/module.c` the following
 *    sets the logging module name to `'module.c'`: 
 *    `SETDEFAULTLOGMODULENAME();`
 *
 * - Logging has to be initialized with a call to `log_init()` first
 *   (and later de-initialized with a call to `log_uninit()`)  
 *   before logging can be used. 
 *
 * - After this the following macros can be used in your code:
 *   - `LOG(log_level, format_string, ...)`
 *     - Logs to console and file with the specified `log_level`.
 *     - Works like printf: __Example__: 
 *       - `LOG(log_ERROR, "Cannot find index %i", index);`
 *
 *   - `LOG_CONSOLE(log_level, format_string, ...)`:
 *     - Logs to console only with the specified `log_level`.
 *
 *   - `LOG_FILE(log_level, ...)`:
 *     - Logs to file only with the specified `log_level`.
 *
 * @{
 * @file log.h Header file.
 */

#include "config.h"

/** Log levels. */
enum {
    log_DISABLED = 0, /*!< Logging disabled. */
    log_ALWAYS   = 1, /*!< Always level. */
    log_ERROR    = 2, /*!< Error level. */
    log_WARNING  = 3, /*!< Warning level. */
    log_INFO     = 4, /*!< Info level. */
    log_DEBUG    = 5, /*!< Debug level. */
    log_VERBOSE  = 6  /*!< Verbose level. */
};

/** Initialize logging. */
int log_init( const char *file, int loglevel_file, int loglevel_console );

/** De-initialize logging. */
void log_deinitialize();

/** Set the log levels for file and console. */
void log_setlevel( int loglevel_file, int loglevel_console );

#if !LOGGING
    /* If logging is disabled at compile time, set log macros to 
     * empty defines -> will replace log statements with noop in code 
     * Doxygen will only detect the documentation here. */

    /** Logs to console and file with the specified `loglevel`.
     * Works like printf: __Example__: 
     * - `LOG(log_ERROR, "Cannot find index %i", index);` */
    #define LOG(loglevel, ...)
    /** Logs to console and file with the specified log levels for file and console.
     * Works like printf: __Example__: 
     * - `LOG(log_WARNING, log_ERROR, "Cannot find index %i", index);` */
    #define LOG2(loglvl, loglvl_cons, ...)
    #define LOG_CONSOLE(loglevel, ...)
    #define LOG_FILE(loglevel, ...)
    #define SETLOGMODULENAME(name)
    #define SETDEFAULTLOGMODULENAME()
#else
    #include <string.h>
    #ifdef _WIN32
        #define DIRSEP '/'
    #else
        #define DIRSEP '\\'
    #endif
    #define _LOG_GETNAME_FUNCTION \
    static const char *_log_getname() { \
        static const char* f__ = __FILE__; \
        static int executed = 0; \
        if( !__log_auto_basename ) return _LOGFILENAME__; \
        if( !executed && _LOGFILENAME__ == f__ ) { \
            _LOGFILENAME__ = strrchr(__FILE__, DIRSEP) ? strrchr(__FILE__, DIRSEP) + 1 : __FILE__; \
            executed = 1; \
        } \
        return _LOGFILENAME__; \
    }

    #define SETLOGMODULENAME(name) static const char* _LOGFILENAME__ = name; \
        static const int __log_auto_basename = 0; _LOG_GETNAME_FUNCTION
    #define SETDEFAULTLOGMODULENAME() static char* _LOGFILENAME__ = __FILE__; \
        static const int __log_auto_basename = 1; _LOG_GETNAME_FUNCTION

    #define LOG(loglvl, ...) write_log(loglvl,loglvl,_log_getname(),__LINE__, __VA_ARGS__)
    #define LOG2(loglvl, loglvl_cons, ...) write_log(loglvl,loglvl_cons,_log_getname(),__LINE__, __VA_ARGS__)
    #define LOG_CONSOLE(loglevel, ...) write_log_console(loglevel,_log_getname(),__LINE__, __VA_ARGS__)
    #define LOG_FILE(loglevel, ...) write_log_file(loglevel,_log_getname(),__LINE__, __VA_ARGS__)
#endif

/** Logs to file and console with different log levels. */
void write_log( int loglevel_file, int loglevel_console,
                  const char* filename, const unsigned long line, const char* format, ...);

/** Log to file. */
void write_log_file( int loglevel_file, const char* filename,
                        const unsigned long line, const char* format, ...);

/** Log to console */
void write_log_console( int loglevel_console, const char* filename,
                           const unsigned long line, const char* format, ...);

/** @} */                           
                           
#endif /* LOG_H_ */
