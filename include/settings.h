#ifndef SETTINGS_H_
#define SETTINGS_H_

/** @defgroup settings Settings
 * Functionality to read the server settings from a configuration file.
 * Read an settings from a file, the following settings are supported 
 * (lines starting with `#` are optional and have default values).
 * Some settings can be overwritten via command line options.
 *
 * Example configuration file:
 * @code{.ini}
 * [server]
 * # wwwroot= www root directory, none by default
 * # port= server port, 8181 default
 * # logfile = logfile, ~/cranberry-server.log by default
 * # loglevel_file = 0 for off, 1-6 for log levels (always, error, warning, info, debug, verbose) 
 *                    -  2 by default (warning)
 * # loglevel_console = 0 for off, 1-6 for log levels (always, error, warning, info, debug, verbose) 
 *                       -  1 by default (error)
 * # ipv6 = 1 or 0, 1 by default / enables or disables ipv6 support
 * # deflate = 0-9, 0 is off = default, 1-9 is compression level, 
 *                              while 1 is the fastest and 9 the best compression
 * # disable_embedded_res = 0 or 1, 0 by default
 *
 * # [scripting]
 * # enabled = 0 or 1,  1 by default
 * # error_output_socket = 1 or 0, 1 by default
 * # session_timeout = ..     1800 by default
 * # caching = 0 or 1, 0 by default
 * 
 * # [scripting_cache]  ; only used if scripting.caching = 1
 * # cache_tmpfile = 0 or 1, 1 by default (if scripting caching =1)
 * # cache_memory = 0 or 1, 0 by default
 * # cache_memory_limit_mb = [max cache size in mb], 10 by default
 * # cache_tmpfile_limit_mb = [max cache size in mb], 50 by default
 * @endcode
 * 
 * @{
 * @file settings.h Header file. 
 */
 
#define WEBSRV_PORT_NOT_SET  -1
#define SETTING_VAL_NOT_SET  WEBSRV_PORT_NOT_SET

#if LUA_SUPPORT
    enum {
        LUASP_CACHING_NONE = 0,        /**< Lua caching off. */
        LUASP_CACHING_FILE = 1 << 0,   /**< Lua caching to temporary directory. */
        LUASP_CACHING_MEMORY = 1 << 1  /**< Lua caching to memory. */
    };

    typedef struct {
        /** Lua scripting activated/deactivated, default is 1 (enabled) */
        int enabled;
        /** Error output of scripts to http socket on or off. The default is 1 (enabled) */
        int error_output_socket;
        /** Web session time out in seconds, default is 1800 (30min) */
        unsigned session_timeout;
        /** Lua caching enabled/disabled, default is LUASP_CACHING_NONE (0) */
        int cache; /*  NOT YET SUPPORTED */
    } scripting_t;
#endif

/** Web server settings */
typedef struct {
    int port;              /**< Server listen port. */
    char *wwwroot;         /**< www root dir where html/css/js files are stored. */
    int ipv6;              /**< IPv6 enabled(1)/disabled(0), the default is enabled (1) */

    int loglevel_file;     /**< Log level for log file. The default is log_WARNING. */
    int loglevel_console;  /**< Log level for console. The default is log_ERROR. */
    char *logfile;         /**< The log output file */

    int disable_er;        /**< Disable embedded resource lookups if 1 - default is 0. */

#ifdef DEFLATE_SUPPORT
    int deflate;           /**< Auto compress static content for compressible 
                                MIME types with deflate if the client supports it.
                                The default is off (0). */
#endif

#if LUA_SUPPORT
    scripting_t scripting; /**< Scripting settings. */
#endif
} server_settings_t;

/** Initialize settings. */
server_settings_t * settings_init( void );

/** Deinitialize settings. */
void settings_free( server_settings_t *pSettings );

/** Load settings from a configuration file. */
int settings_loadini( server_settings_t *pSettings, const char *filename, int DefaultOnError );

/** @} */

#endif /* SETTINGS_H_ */
