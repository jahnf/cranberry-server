/* cranberry-server. A small C web server application with lua scripting, 
 * session and sqlite support. https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
#define LUA_SUPPORT 1
#if LUA_SUPPORT
/**
 *
 *
 */

#include "luasp_cache.h"
#include "cfile.h"
#include "settings.h"
#include "cthreads.h"

#include "log.h"
//SETLOGMODULENAME("lsp_cache")

#include <lua.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "cresource.h"

enum {
    LSPC_ITEM_TYPE_TMPFILE = 1,
    LSPC_ITEM_TYPE_MEMORY  = 2
};

//typedef int (*lua_Writer) (lua_State *L,
//                           const void* p,
//                           size_t sz,
//                           void* ud);

#define CACHE_HASH_TABLE_PWR 9
#define CACHE_HASH_TABLE_SIZE (1<<CACHE_HASH_TABLE_PWR) /*512*/
#define CACHE_HASH_MODULO (CACHE_HASH_TABLE_SIZE - 1)

typedef struct memblock_t_struct memblock_t;
struct memblock_t_struct {
    size_t size;
    void *data;
    memblock_t *next;
};

typedef struct {
    size_t size;
    memblock_t *data;
    time_t time;    /* creation time, last update time */
} luasp_cache_item_mem_t;

typedef struct {
    FILE *fp;
    char *filename; /* tmpfile filename */
    size_t filesize;
    time_t time;
} luasp_cache_item_file_t;

typedef struct luasp_cache_item_t_struct luasp_cache_item_t;
struct luasp_cache_item_t_struct {
    int type;           /* item type */
    char *filename;     /* web filename */
    void *data;
    luasp_cache_item_t *next;
};

typedef struct {
    char *tempdir;
    c_rwlock rwlock;
    luasp_cache_item_t *cache_items[CACHE_HASH_TABLE_SIZE];
    size_t size_mem;    /* total memory cache size */
    size_t size_file;   /* total tmpfile cache size */
} luasp_cache_t;

typedef struct {
    luasp_cache_item_file_t *cache_item;

} cache_file_writer_ud_t;




static int luasp_cache_file_writer(lua_State *L, const void* p, size_t sz, void *ud ) {

    //cache_file_writer_ud_t *cwud = ud;
    //luasp_cache_item_file_t *cf_item = ((cache_file_writer_ud_t*)ud)->cache_item;

    //cf_item->fp; // needs to be non-NULL and opened
//    cf_item->filesize; // usually 0 on first call with ud
//    cf_item->filename; // is set
//    cf_item->time; // is set


    return 0;
}

int luasp_cache_add_lua_State(void *pcache, lua_State *L, const char *name ) {

    //luasp_cache_t *cache = (luasp_cache_t*)pcache;
    /* check if entry for name already exists */
    /* automatically select type of cache , tmpfile or memory */

    if( /*tmpfile*/ 1 ) {
        int ret = 0;
        cache_file_writer_ud_t ud;
        ud.cache_item = (luasp_cache_item_file_t*)malloc(sizeof(luasp_cache_item_file_t));
        /*TODO check for allocation error*/
        /* TODO create tempfile, open tempfile
         *
         */
        ud.cache_item->filesize = 0;
        // ud.cache_item->fp = fp;
        ud.cache_item->time = time(NULL);
        //ud.cache_item->filename = copy the name

        ret = lua_dump( L, luasp_cache_file_writer, &ud, 1 );
        if( ret == 0 ) {
            /* add entry to cache if not exists */
            int b = 0;
            if( b ) {
#ifdef _WIN32
            //mkstemp("/tmp/filefileXXXXXXX");
#else
                mkstemp("/tmp/filefileXXXXXXX");
#endif
            }
        }
    }



    return 0;
}

void *luasp_cache_init( thread_arg_t *args ) {

//	luasp_cache_t *cache;
//    const WebSrvSettings *pSettings = (const WebSrvSettings*)args->pSettings;
//    if( !pSettings->scripting.cache ) return NULL;
//
//    cache = (luasp_cache_t*)malloc(sizeof(luasp_cache_t));
//
//    if( cache ) {
//        /* init tmpfile caching if enabled */
//        if( pSettings->scripting.cache & LUASP_CACHING_FILE ) {
//            const char *tempdir;
//            if( (tempdir = cfile_get_tempdir()) ) {
//                if( (cache->tempdir = (char*)malloc(strlen(tempdir)+1)) ) {
//                    strcpy( cache->tempdir, tempdir );
//                } else {
//                    LOG( args, log_WARNING, "malloc failed" );
//                }
//            } else {
//                LOG( args, log_WARNING, "could not set temp dir" );
//            }
//
//            if( cache->tempdir ) {
//                /* do more stuff :) */
//            }
//        }
//
//        /* if tmpfile caching and memory caching is not set */
//        if( !cache->tempdir && !(pSettings->scripting.cache & LUASP_CACHING_MEMORY) ) {
//            free(cache);
//            return NULL;
//        }
//
//        /* initialize cache members */
//        cthread_rwlock_init( &cache->rwlock, 20 );
//        cache->size_mem = 0;
//        cache->size_file = 0;
//        memset( cache->cache_items, 0, sizeof(cache->cache_items) );
//
//    }
//    return cache;
    return NULL;
}

void luasp_cache_free( void *pCache ) {
    luasp_cache_t *cache = pCache;
    if( cache == NULL ) return;

    free( cache->tempdir );
    free( cache );
}

#endif  /* LUA_SUPPORT */
