/** @file cmdline.h
 *	@author Jahn Fuchs
 *
 */

#ifndef VERSION__H_
#define VERSION__H_

enum {
    BUILD_TYPE_UNKNOWN = 0,
    BUILD_TYPE_RELEASE = 1,
    BUILD_TYPE_DEBUG = 2
};

#ifndef ___VERSION_C_
const extern unsigned RevisionNumber;
const extern int      BuildType;
#endif

#endif /* VERSION__H_ */
