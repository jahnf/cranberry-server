/** @file version.h */

#ifndef VERSION__H_
#define VERSION__H_

enum {
    BUILD_TYPE_UNKNOWN = 0,  /*!< Unknown build type. */
    BUILD_TYPE_RELEASE = 1,  /*!< Release build type. */
    BUILD_TYPE_DEBUG = 2     /*!< Debug build type. */
};

/** Return the server version string. 
 * Usually created by the build system post build with 
 * information from the version control system (git in this case) */
const char* get_version_string();

/** Return the build type, one of BUILD_TYPE_XX values. */
int get_build_type();

#endif /* VERSION__H_ */
