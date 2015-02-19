/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
#ifndef CFILE_H_
#define CFILE_H_

/** @defgroup cfile CFile
 * General purpose Windows/Linux file functions.
 * @{
 * @file cfile.h
 */

#include <stddef.h>
#include <sys/types.h>

#ifdef _WIN32
    /** Directory separator character. */
    #define DIR_SEP '\\'
    /** Directory separator string. */
    #define DIR_SEP_STR "\\"
#else
    /** Directory separator character. */
    #define DIR_SEP '/'
    /** Directory separator string. */
    #define DIR_SEP_STR "/"
#endif

/** CFILE return codes. */
enum _CFILE_RETCODES {
    CFILE_SUCCESS = 0,      /**< Success. */
    CFILE_DOES_NOT_EXIST,   /**< File does not exist. */
};

/** CFILE types. */
enum _CFILE_TYPES {
    CFILE_TYPE_UNKNOWN = 0,   /**< Unknown type. */
    CFILE_TYPE_REGULAR,       /**< Regular file. */
    CFILE_TYPE_DIR,           /**< Directory. */
    CFILE_TYPE_DRIVE,         /**< Drive (Windows specific). */
    CFILE_TYPE_LNKFILE,       /**< Link to a file. */
    CFILE_TYPE_LNKDIR,        /**< Link to a directory. */
    CFILE_TYPE_LINK           /**< Link. */
};

/** CFILE item access rights. */
enum _CFILE_ACCESS {
    CFILE_ACCESS_UNDEFINED = 0,     /**< Undefined. */
    CFILE_ACCESS_RO,                /**< Read only. */
    CFILE_ACCESS_RW,                /**< Read and write. */
    CFILE_ACCESS_WO,                /**< Write only. */
    CFILE_ACCESS_NA                 /**< Not accessible 
                                     * (i.e. can be listed but not 
                                     *  read or written) */
};

typedef struct {
    off_t size;
    short type;
} cfile_stat_t;

/** File item. @see struct cfile_item_t */
typedef struct cfile_item_t cfile_item_t;

/** File item. */
struct cfile_item_t {
    char *name;     /**< File name. */
    short access;   /**< Access right (one of _CFILE_ACCESS). */
    short type;     /**< File item type (one of _CFILE_TYPE). */
    off_t size;     

    cfile_item_t *parent;   /**< Parent item */
    cfile_item_t *children; /**< Children - always NULL for
                                   regular files, pointer to self 
                                   for empty directories */
    cfile_item_t *next;     /**< Next item */
};

/** Get temporary directory. Returns NULL if the temporary
 * directory could not be determined. This function is not thread safe. */
const char * cfile_get_tempdir();

/** Get home directory of current user. Returns NULL if the 
 * home directory could not be determined. This function is not thread safe. */
const char * cfile_get_homedir();

/** Return file information. */
int cfile_getstat( const char *filename, cfile_stat_t *st );

/** Frees the given file item, all the next items and children (but NOT the parent) */
void cfile_item_free( cfile_item_t *cf_item );

/** Get all drives or root nodes. On *nix this there is just one root node, 
 * on Windows all logical drives are considered a root node. */
cfile_item_t * cfile_get_drives();

/** Get a directory list. */
cfile_item_t * cfile_list_dir( const char *path, cfile_item_t *parent );

/** @} */

#endif /* CFILE_H_ */
