/** @file settings.h
 *	@author Jahn Fuchs
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#define WEBSRV_PORT_NOT_SET  -1
#define SETTING_VAL_NOT_SET  WEBSRV_PORT_NOT_SET

#if LUA_SUPPORT
    enum {
        LUASP_CACHING_NONE = 0,
        LUASP_CACHING_FILE = 1 << 0,
        LUASP_CACHING_MEMORY = 1 << 1
    };

    typedef struct {
        /* lua scripting activate/deactivate switch, default is 1 (enabled) */
        int enabled;
        /* 0 if errors should not be printed via http, default is 1 (enabled) */
        int error_output;
        /* session timeout in seconds, default is 1800 (30min) */
        unsigned session_timeout;
        /* if caching is enabled, default is LUASP_CACHING_NONE (0) */
        int cache; /*  NOT YET SUPPORTED */
    } scripting_t;
#endif

typedef struct {
    int port;					// listen port
    char *wwwroot;				// www root dir where html/css/js files are stored.
    int ipv6;					// 0 if ipv6 is disabled // default is 1

    int loglevel_file;			// 0 for off, everything else for on, default is log_WARNING
    int loglevel_console;		// 0 for off, everything else for on, default is log_ERROR
    char *logfile;				// file where to write log

    int disable_er;				// 1 for disabling embedded resources, default is 0

#ifdef DEFLATE_SUPPORT
    int deflate;				// auto compress static content for compressable mimetypes with deflate
                                // if client supports it, default is 0.
#endif

    //-- scripting settings -------------------------------------------------
#if LUA_SUPPORT
    scripting_t scripting;
#endif

} server_settings_t;

/** Initialize settings */
server_settings_t * settings_init( void );
/** Deinitialize settings */
void settings_free( server_settings_t *pSettings );
/** Load settings from a file */
int settings_loadini( server_settings_t *pSettings, const char *filename, int DefaultOnError );

#endif /* SETTINGS_H_ */
