/** @file settings.h
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#define WEBSRV_PORT_NOT_SET  -1
#define SETTING_VAL_NOT_SET  WEBSRV_PORT_NOT_SET

#if LUA_SUPPORT
    enum {
        LUASP_CACHING_NONE = 0,         /**< Lua caching off. */
        LUASP_CACHING_FILE = 1 << 0,    /**< Lua caching to temporary directory. */
        LUASP_CACHING_MEMORY = 1 << 1   /**< Lua caching to memory. */
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

#endif /* SETTINGS_H_ */
