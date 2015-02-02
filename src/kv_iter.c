/**
	TODO mention stringencoders. documentation
*/

#include "kv_iter.h"

void kviter_reset( kviter_t* kvi,
		const char key_sep, const char val_sep,
		const char* s, size_t len)
{ 
	kvi->key_sep = key_sep;
	kvi->val_sep = val_sep;

    kvi->s = s;
    kvi->len = len;
    kvi->pos = 0;

    kvi->key = NULL;
    kvi->keylen = 0;
    kvi->val = NULL;
    kvi->vallen = 0;
}

int kviter_next( kviter_t* kvi ) { 
	return kviter_next_i( kvi, 0, 0);
}

int kviter_next_i( kviter_t* kvi, int ignore_leading_char, char c )
{
    if (kvi->pos >= kvi->len) {
    	kvi->key = NULL;
    	kvi->keylen = 0;
    	kvi->val = NULL;
    	kvi->vallen = 0;
        return 0;
    }
	{
		char *ends;
		const char* charstart = kvi->s + kvi->pos;
		if( ignore_leading_char ) {
    		while( *charstart == c ) {
    			++charstart;
    			++kvi->pos;
    		}
		}

		ends = (char*) memchr(charstart, kvi->key_sep, kvi->len - kvi->pos);

		if (ends == NULL) {
			char* eq = (char*) memchr(charstart, kvi->val_sep, kvi->len - kvi->pos);
			if (eq == NULL) {
				kvi->key = charstart;
				kvi->keylen = kvi->len - kvi->pos;
				kvi->val = NULL;
				kvi->vallen = 0;
			} else {
				kvi->key = charstart;
				kvi->keylen = eq - charstart;
				kvi->val = eq + 1;
				kvi->vallen = (kvi->s + kvi->len) - kvi->val;
			}
			kvi->pos = kvi->len;
			return 1;
		} else {
			// &&foo=bar
			char* eq = (char*) memchr(charstart, kvi->val_sep, ends - charstart);
			if (eq == NULL) {
				kvi->key = charstart;
				kvi->keylen = ends - charstart;
				kvi->val = NULL;
				kvi->vallen = 0;
			} else {
				kvi->key = charstart;
				kvi->keylen = eq - charstart;
				kvi->val = eq + 1;
				kvi->vallen = ends - eq - 1;
			}
			kvi->pos = (ends - kvi->s) + 1;
			return 1;
		}
	}
}
