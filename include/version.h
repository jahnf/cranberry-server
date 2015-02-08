/** @file version.h
 *
 */

#ifndef VERSION__H_
#define VERSION__H_

enum {
    BUILD_TYPE_UNKNOWN = 0,
    BUILD_TYPE_RELEASE = 1,
    BUILD_TYPE_DEBUG = 2
};

const char* get_version_string();
int get_build_type();

#endif /* VERSION__H_ */
