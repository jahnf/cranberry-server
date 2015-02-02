/*
 * cfile.h
 * general purpose win/linux file functions
 *
 */

#ifndef CFILE_H_
#define CFILE_H_

#include <stddef.h>
#include <sys/types.h>

#ifdef _WIN32
    #define DIR_SEP '\\'
    #define DIR_SEP_STR "\\"
#else
    #define DIR_SEP '/'
    #define DIR_SEP_STR "/"
#endif

enum _CFILE_RETCODES {
	CFILE_SUCCESS = 0,
	CFILE_DOES_NOT_EXIST,
};

enum _CFILE_TYPES {
	CFILE_TYPE_UNKNOWN = 0,
	CFILE_TYPE_REGULAR,
	CFILE_TYPE_DIR,
	CFILE_TYPE_DRIVE,
	CFILE_TYPE_LNKFILE,		// link to file
	CFILE_TYPE_LNKDIR,		// link to dir
	CFILE_TYPE_LINK
};

enum _CFILE_ACCESS {
	CFILE_ACCESS_UNDEFINED = 0,
	CFILE_ACCESS_RO,	// read only
	CFILE_ACCESS_RW,	// read & write
	CFILE_ACCESS_WO,	// write only
	CFILE_ACCESS_NA 	// not accessible (i.e. can be listed but not read or written)
};

typedef struct {
	off_t size;
	short type;
} cfile_stat_t;


typedef struct s_CFileItem cfile_item_t;

struct s_CFileItem {
	char *name;		// filename
	short access;	// one of _CFILE_ACCESS
	short type;		// one of _CFILE_TYPE
	off_t size;

	cfile_item_t *parent;		// parent item
	cfile_item_t *children;	// always NULL for regular files, pointer to self for empty directories
	cfile_item_t *next;
};


const char * cfile_get_tempdir();
const char * cfile_get_homedir();

int cfile_getstat( const char *filename, cfile_stat_t *st );

// frees the given file item, next items and the childrens (NOT the parent)
void cfile_item_free( cfile_item_t *cf_item );

// file & directory listing...
cfile_item_t * cfile_get_drives();
cfile_item_t * cfile_list_dir( const char *path, cfile_item_t *parent );

#endif /* CFILE_H_ */
