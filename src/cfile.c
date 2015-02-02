
#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <dirent.h>
	#include <limits.h>
    #include <pwd.h>
#endif

#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cfile.h"

/* returns NULL if tempdir could not be determined
 * NOT THREAD SAFE */
const char * cfile_get_tempdir() {
    static const char * tempdir = NULL;
    #ifdef _WIN32
        static char tmp_fallback[MAX_PATH+1];
    #else
        static const char tmp_fallback[] = "/tmp";
    #endif

        if( !tempdir ) {
            tempdir = getenv("TEMP");
            if(!tempdir )
                tempdir = getenv("TMP");
            if(!tempdir )
                tempdir = getenv("TMPDIR");
#ifdef _WIN32
            if(!tempdir) {
                DWORD v = GetTempPath( MAX_PATH, tmp_fallback);
                if( v && v < sizeof(tmp_fallback) )
                    tempdir = tmp_fallback;
            } else if( strlen(tempdir) < sizeof(tmp_fallback) ) {
                strcpy( tmp_fallback, tempdir );
                tempdir = tmp_fallback;
            }
#else
            if(!tempdir)
                tempdir = tmp_fallback;
#endif
        }

    return tempdir;
}


/* returns NULL if homedir could not be determined
 * NOT THREAD SAFE */
const char * cfile_get_homedir() {
    static const char * homedir = NULL;
    #ifdef _WIN32
        char *homedrive = 0, *homepath = 0;
        static char homedir_win[2048];
    #else
        struct passwd *pwd;
        static const char hd_fallback[] = "/";
    #endif

    #ifdef _WIN32
        if( !homedir ) {
            homedrive = getenv("HOMEDRIVE");
            if(!homedrive) homedrive = "C:";

            homepath = getenv("HOMEPATH");

            strcpy(homedir_win, homedrive);
            if(homepath) strcpy(&homedir_win[strlen(homedrive)], homepath);

            homedir = homedir_win;
        }
    #else
        if( !homedir ) {
            homedir = getenv("HOME");

            if(!homedir) {
                pwd = getpwuid(getuid());
                if(pwd && pwd->pw_dir)
                    homedir = pwd->pw_dir;
            }

            if(!homedir) homedir = hd_fallback;
        }
    #endif

    return homedir;
}



int cfile_getstat( const char *filename, cfile_stat_t *cst ) {

	struct stat st;
	if( stat( filename, &st ) != 0 ) {
		return CFILE_DOES_NOT_EXIST;
	}
	cst->size = st.st_size;

	if( st.st_mode & S_IFDIR )
		cst->type = CFILE_TYPE_DIR;
	else if( st.st_mode & S_IFREG )
		cst->type = CFILE_TYPE_REGULAR;
	else
		cst->type = CFILE_TYPE_UNKNOWN;

	return CFILE_SUCCESS;
}


void cfile_item_free( cfile_item_t *cf_item ) {
	cfile_item_t *tmp;
	while( cf_item ) {
		if( cf_item != cf_item->children )
			cfile_item_free( cf_item->children );
		tmp = cf_item;
		cf_item = cf_item->next;
		free( tmp->name );
		free( tmp );
	}
}

cfile_item_t * cfile_item_prepend( const char *name, cfile_item_t *parent, cfile_item_t *prependto ) {
	cfile_item_t *item;
	if( (item = malloc( sizeof(cfile_item_t) )) ) {
		memset( item, 0, sizeof(cfile_item_t) );
		if( name ) {
			item->name = malloc( strlen(name) + 1 );
			strcpy( item->name, name );
		}
		item->next = prependto;
	}
	return item;
}

cfile_item_t * cfile_item_new( const char *name, cfile_item_t *parent ) {
	return cfile_item_prepend( name, parent, NULL );
}

static int _stat_to_cfile_attr( struct stat *fattr ) {
	unsigned int drwx[4] = {0,0,0,0};
	drwx[0] = fattr->st_mode & S_IFDIR;
#ifdef _WIN32
	drwx[1] = fattr->st_mode & S_IREAD;
	drwx[2] = fattr->st_mode & S_IWRITE;
	drwx[3] = fattr->st_mode & S_IEXEC;
#else

	drwx[1] = fattr->st_mode & S_IROTH;
	drwx[2] = fattr->st_mode & S_IWOTH;
	drwx[3] = fattr->st_mode & S_IXOTH;

	if( !drwx[0] && drwx[1] && drwx[2] ) return CFILE_ACCESS_RW;
	if( drwx[0] && drwx[1] && drwx[2] && drwx[3] ) return CFILE_ACCESS_RW;

	if( fattr->st_gid == getegid() ) {
		drwx[1] |= fattr->st_mode & S_IRGRP;
		drwx[2] |= fattr->st_mode & S_IWGRP;
		drwx[3] |= fattr->st_mode & S_IXGRP;
	}

	if( !drwx[0] && drwx[1] && drwx[2] ) return CFILE_ACCESS_RW;
	if( drwx[0] && drwx[1] && drwx[2] && drwx[3] ) return CFILE_ACCESS_RW;

	if( fattr->st_uid == geteuid() ) {
		drwx[1] |= fattr->st_mode & S_IRUSR;
		drwx[2] |= fattr->st_mode & S_IWUSR;
		drwx[3] |= fattr->st_mode & S_IXUSR;
	}

#endif

	if( !drwx[0] ) {
		if( drwx[1] ) {
			if( drwx[2] ) return CFILE_ACCESS_RW;
			return CFILE_ACCESS_RO;
		}
		if( drwx[2] ) return CFILE_ACCESS_WO;
	}
	else if( drwx[3] ) {
		if( drwx[1] ) {
			if( drwx[2] ) return CFILE_ACCESS_RW;
			return CFILE_ACCESS_RO;
		}
		if( drwx[2] ) return CFILE_ACCESS_WO;
	}
	return CFILE_ACCESS_NA;
}

static int ___compare (const char *a, const char *b)
{
	// dont use first dot for sorting...:
	if( *a == '.' ) ++a;
	if( *b == '.' ) ++b;

	#ifdef _WIN32
		return _stricmp( a, b );
	#else
		return strcasecmp( a, b );
	#endif
}


// file & directory listing...
cfile_item_t * cfile_list_dir( const char *path, cfile_item_t *parent ) {
	cfile_item_t *new_list = NULL;
	size_t path_len = strlen(path);
	int path_has_trailing_slash = path_len?(path[path_len-1] == DIR_SEP):0;
	#ifdef _WIN32
		char buf[MAX_PATH];
		HANDLE hFind = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATA ffd;

		strcpy( buf, path );
		if( !path_has_trailing_slash ) {
			strcpy( &buf[path_len], DIR_SEP_STR );
			++path_len;
		}
		strcpy( &buf[path_len], "*" );

		hFind = FindFirstFile( buf, &ffd );
		if (INVALID_HANDLE_VALUE != hFind) {
			cfile_item_t *prev = NULL;
			do
			{
				cfile_item_t *new_item = cfile_item_new( ffd.cFileName, parent );

	#else /* linux, posix ...*/
		DIR *dp = opendir( path );
		if (dp != NULL)
		{
			struct dirent *ep;
			char buf[PATH_MAX];
			strcpy( buf, path );
			if( !path_has_trailing_slash ) {
				strcpy( &buf[path_len], DIR_SEP_STR );
				++path_len;
			}

			while( (ep = readdir(dp)) ) {

				cfile_item_t *new_item = cfile_item_new( ep->d_name, parent );
	#endif
				if( new_list == NULL ) {
					new_list = new_item;
				} else {
					cfile_item_t *it = new_list, *prev = NULL;
					for( ; it; prev=it, it=it->next ) {
						if( ___compare( new_item->name, it->name ) < 0 ) {
							new_item->next = it;
							if( it == new_list ) new_list = new_item;
							else prev->next = new_item;
							break;
						}
					}
					if( it == NULL ) prev->next = new_item;
				}
	#ifdef _WIN32
				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					new_item->type = CFILE_TYPE_DIR;
				}
				else
				{
					//filesize.LowPart = ffd.nFileSizeLow;
					//filesize.HighPart = ffd.nFileSizeHigh;
					printf("@  %s   xbytes\n", ffd.cFileName/*, filesize.QuadPart*/);
					new_item->type = CFILE_TYPE_REGULAR;
				}
				prev = new_item;

				{
					struct stat fattr;
					strcpy( buf, path );
					if( !path_has_trailing_slash ) strcat( buf, DIR_SEP_STR );
					strcat( buf, new_item->name );

					if( 0 == stat( buf, &fattr ) ) {
						new_item->access = _stat_to_cfile_attr(&fattr);
						if( new_item->type == CFILE_TYPE_REGULAR || new_item->type == CFILE_TYPE_LNKFILE ) {
							new_item->size = fattr.st_size;
						}
					}
					else new_item->access = CFILE_ACCESS_NA;
				}
			}
			while (FindNextFile(hFind, &ffd) != 0);
			FindClose(hFind);

		} else /*findfirst failure*/;
	#else /* linux ... */
				switch( ep->d_type ){
				case DT_DIR:	new_item->type = CFILE_TYPE_DIR; break;
				case DT_LNK:	new_item->type = CFILE_TYPE_LINK; break;
				case DT_REG:	new_item->type = CFILE_TYPE_REGULAR; break;
				default:
					new_item->type = CFILE_TYPE_UNKNOWN; // a file type we dont support
					break;
				}
				// read in basic file attributes...
				{
					struct stat fattr;
					strcpy( &buf[path_len], new_item->name );

					if( 0 == stat( buf, &fattr ) ) {
						new_item->access = _stat_to_cfile_attr(&fattr);

						if( new_item->type != CFILE_TYPE_UNKNOWN ) {
							if( new_item->type == CFILE_TYPE_LINK ) {
								if( fattr.st_mode & S_IFDIR )
									new_item->type = CFILE_TYPE_LNKDIR;
								else if( fattr.st_mode & S_IFREG )
									new_item->type = CFILE_TYPE_LNKFILE;
							}
						} else {
							if( fattr.st_mode & S_IFDIR ) {
								if( fattr.st_mode & S_IFLNK )
									new_item->type = CFILE_TYPE_LNKDIR;
								else
									new_item->type = CFILE_TYPE_DIR;
							} else if( fattr.st_mode & S_IFREG ) {
								if( fattr.st_mode & S_IFLNK )
									new_item->type = CFILE_TYPE_LNKFILE;
								else
									new_item->type = CFILE_TYPE_REGULAR;
							}
						}
						if( new_item->type == CFILE_TYPE_REGULAR || new_item->type == CFILE_TYPE_LNKFILE ) {
							new_item->size = fattr.st_size;
						}
					}
					else new_item->access = CFILE_ACCESS_NA;
				}
			}
		}
		(void) closedir (dp);
	#endif

	return new_list;
}

// retreive a list of drives or root nodes..
// on unix this there is just one root node, on
// windows all logical drives are considered a root node..
cfile_item_t * cfile_get_drives() {
	cfile_item_t *new_list = NULL;
	#ifdef _WIN32
		#define BUFFERSIZE 4096
		char buf[BUFFERSIZE];
		struct stat fattr;
		size_t i, ret =  GetLogicalDriveStrings( BUFFERSIZE, buf );
		cfile_item_t *new_item, *prev = NULL;
		for( i = 0; i < ret; ++i ) {
			if( 0 == stat( &buf[i], &fattr ) ) {
				new_item = cfile_item_new( &buf[i], NULL );
				if( new_list == NULL ) new_list = new_item;
				else prev->next = new_item;

				new_item->access = _stat_to_cfile_attr(&fattr);
				new_item->type = CFILE_TYPE_DRIVE;
				prev = new_item;
			}
			i+=strlen( &buf[i] );
		}
	#else /* linux ...*/
		struct stat fattr;
		if( 0 == stat( "/", &fattr ) ) {
			new_list = cfile_item_new( "/", NULL );
			new_list->access = _stat_to_cfile_attr(&fattr);
			new_list->type = CFILE_TYPE_DRIVE;
		}
	#endif

	return new_list;
}


