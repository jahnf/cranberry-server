/* cranberry-server. A small C web server application with lua scripting,
 * session and sqlite support. https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */

/** @addtogroup logging
 * @{
 * @file log.c Source file.
 */

#include "log.h"
#include "cthreads.h"

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>

    #if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
      #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
    #else
      #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
    #endif

    #ifndef _TIMEZONE_DEFINED /* also in sys/time.h */
    #define _TIMEZONE_DEFINED
    struct timezone
    {
        int  tz_minuteswest; /* minutes W of Greenwich */
        int  tz_dsttime;     /* type of dst correction */
    };
    #endif /* _TIMEZONE_DEFINED */

    int gettimeofday(struct timeval *tv, struct timezone *tz);
#else
    #include <sys/time.h>
#endif

#define LOGGING_LOG_DATETIME             1
#ifndef NDEBUG
#define LOGGING_LOG_LINE                 1
#endif
#define LOGGING_LOG_FILE                 1
#define LOGGING_LOG_LEVELSTRING_FILE     1
#define LOGGING_LOG_LEVELSTRING_CONSOLE  1

/** Internally used settings struct. */
typedef struct {
    int initialized;

    int loglevel_file;
    int loglevel_console;
    FILE *file;

    /* log mutexes */
    c_mutex log_mutex;
    c_mutex print_mutex;
} log_settings_t;

#if LOGGING
static log_settings_t LogSettings = {0};

static const struct {
    const char *str;
    int ikey;
} _log_levels[] = {
    { NULL, 0 },
    { "always ", log_ALWAYS },
    { "error  ", log_ERROR },
    { "warning", log_WARNING},
    { "info   ", log_INFO },
    { "debug  ", log_DEBUG },
    { "verbose", log_VERBOSE },
    { NULL, 0 }
};
#endif

int log_init( const char *file, int loglevel_file, int loglevel_console )
{
#if LOGGING
    cthread_mutex_init( &LogSettings.log_mutex );
    cthread_mutex_init( &LogSettings.print_mutex );

    LogSettings.loglevel_file = (file ? loglevel_file:log_DISABLED);
    LogSettings.loglevel_console = loglevel_console;

    /* if log level for files > log_DISABLED,
     * open file for writing (append mode) */
    if( LogSettings.loglevel_file > log_DISABLED ) {
        LogSettings.file = fopen(file, "a");
        if( !LogSettings.file )  {
            fprintf(stderr, "cannot open log file, logging is now disabled...\n");
            LogSettings.loglevel_file = log_DISABLED;
        }
    }
    LogSettings.initialized = 1;
#endif
    return 0;
}

static void _log_to_file(log_settings_t *pLogSettings, const char* type, const char *filename,
                         const unsigned long line, const char* format, va_list args)
{
#if LOGGING

    cthread_mutex_lock( &pLogSettings->log_mutex );

    #if LOGGING_LOG_DATETIME
    {
        struct timeval curtime;
        struct tm * timeinfo;
        time_t rawtime;
        gettimeofday ( &curtime, NULL);
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        fprintf( pLogSettings->file, "%04d-%02d-%02d %02d:%02d:%02d.%06ld ",
                                    timeinfo->tm_year + 1900,
                                    timeinfo->tm_mon + 1,
                                    timeinfo->tm_mday,
                                    timeinfo->tm_hour,
                                    timeinfo->tm_min,
                                    timeinfo->tm_sec,
                                    curtime.tv_usec);
    }
    #endif

    #if LOGGING_LOG_LEVELSTRING_FILE
        fprintf( pLogSettings->file, "(%s) ", type);
    #endif

    #if LOGGING_LOG_FILE
        #if LOGGING_LOG_LINE
            if( filename ) fprintf( pLogSettings->file, "%s(%lu): ", filename, line);
        #else
            if( filename ) fprintf( pLogSettings->file, "%s: ", filename);
        #endif
    #endif
    vfprintf( pLogSettings->file, format, args );
    fwrite ("\n", 1, 1, pLogSettings->file);
    fflush( pLogSettings->file );

    cthread_mutex_unlock( &pLogSettings->log_mutex );

#endif
}

static void _log_to_console( log_settings_t *pLogSettings, const int loglevel,
                             const char* type, const char *filename,
                             const unsigned long line, const char* format, va_list args )
{
#if LOGGING
    FILE *output = stdout;
    if(loglevel == log_ERROR)
        output = stderr;

    cthread_mutex_lock( &pLogSettings->print_mutex );

    #if LOGGING_LOG_DATETIME
    {
        struct timeval curtime;
        struct tm * timeinfo;
        time_t rawtime;
        gettimeofday ( &curtime, NULL);
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        fprintf( output, "%04d-%02d-%02d %02d:%02d:%02d.%06ld ",
                                    timeinfo->tm_year + 1900,
                                    timeinfo->tm_mon + 1,
                                    timeinfo->tm_mday,
                                    timeinfo->tm_hour,
                                    timeinfo->tm_min,
                                    timeinfo->tm_sec,
                                    curtime.tv_usec);
    }
    #endif

    #if LOGGING_LOG_LEVELSTRING_CONSOLE
        if( loglevel != log_ALWAYS )
            fprintf( output, "(%s) ", type);
    #endif

    #if LOGGING_LOG_FILE
        #if LOGGING_LOG_LINE
            if( filename ) fprintf( output, "%s(%lu): ", filename, line);
        #else
            if( filename ) fprintf( output, "%s: ", filename);
        #endif
    #endif
    vfprintf( output, format, args );
    fwrite ("\n", 1, 1, output);

    cthread_mutex_unlock( &pLogSettings->print_mutex );
#endif
}

void write_log_file( int loglevel_file, const char* filename, const unsigned long line, const char* format, ...)
{
#if LOGGING
    va_list args;

    if( !LogSettings.initialized ) {
        fprintf( stderr, "ERROR: Logging is not initialized.\n");
        return;
    }

    if( loglevel_file > LogSettings.loglevel_file ) return;
    if( loglevel_file > log_VERBOSE ) loglevel_file = log_VERBOSE;
    va_start (args, format);
    _log_to_file(&LogSettings, _log_levels[loglevel_file].str, filename, line, format, args);
    va_end (args);
#endif
}

void write_log( int loglevel_file, int loglevel_console,
                  const char* filename, const unsigned long line, const char* format, ...)
{
#if LOGGING
    va_list args;

    if( !LogSettings.initialized ) {
        fprintf( stderr, "ERROR: Logging is not initialized.\n");
        return;
    }

    if( !(loglevel_console > LogSettings.loglevel_console) ) {
        if( loglevel_console > log_VERBOSE ) loglevel_console = log_VERBOSE;
        va_start (args, format);
        _log_to_console( &LogSettings, loglevel_console, _log_levels[loglevel_console].str, filename, line, format, args );
        va_end (args);
    }

    if( !(loglevel_file > LogSettings.loglevel_file) ) {
        if( loglevel_file > log_VERBOSE ) loglevel_file = log_VERBOSE;
        va_start (args, format);
        _log_to_file(&LogSettings, _log_levels[loglevel_file].str, filename, line, format, args);
        va_end (args);
    }
#endif
}

void write_log_console( int loglevel_console, const char* filename,
                           const unsigned long line, const char* format, ...)
{
#if LOGGING
    va_list args;

    if( !LogSettings.initialized ) {
        fprintf( stderr, "warning: uninitialized logging\n");
        return;
    }

    if( loglevel_console > log_VERBOSE ) loglevel_console = log_VERBOSE;
    if( loglevel_console > LogSettings.loglevel_console ) return;
    va_start (args, format);
    _log_to_console( &LogSettings, loglevel_console, _log_levels[loglevel_console].str, filename, line, format, args );
    va_end (args);
#endif
}


void log_setlevel( int loglevel_file, int loglevel_console ) {
#if LOGGING
    if( !LogSettings.initialized ) {
        fprintf( stderr, "warning: uninitialized logging\n");
        return;
    }

    LogSettings.loglevel_file = loglevel_file;
    LogSettings.loglevel_console = loglevel_console;
#endif
}

void log_deinitialize()
{
#if LOGGING
    if( !LogSettings.initialized )
        return;

    cthread_mutex_lock(  &LogSettings.print_mutex );
    cthread_mutex_lock(  &LogSettings.log_mutex );

    if( LogSettings.file ) {
        fclose(LogSettings.file);
    }

    cthread_mutex_unlock(  &LogSettings.log_mutex );
    cthread_mutex_destroy( &LogSettings.log_mutex );
    cthread_mutex_unlock(  &LogSettings.print_mutex );
    cthread_mutex_destroy( &LogSettings.print_mutex );

    LogSettings.initialized = 0;
#endif
}


#ifdef _WIN32
#if LOGGING
#if LOGGING_LOG_DATETIME
    int gettimeofday(struct timeval *tv, struct timezone *tz)
    {
      FILETIME ft;
      unsigned __int64 tmpres = 0;
      static int tzflag;

      if (NULL != tv)
      {
        GetSystemTimeAsFileTime(&ft);

        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        /*converting file time to unix epoch*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS;
        tmpres /= 10;  /*convert into microseconds*/
        tv->tv_sec = (long)(tmpres / 1000000UL);
        tv->tv_usec = (long)(tmpres % 1000000UL);
      }

      if (NULL != tz)
      {
        if (!tzflag)
        {
          _tzset();
          tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
      }

      return 0;
    }
#endif
#endif
#endif

/** @} */
