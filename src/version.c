/** @file version.c
 *	@author Jahn Fuchs
 */
#define ___VERSION_C_
#include "version.h"

#ifdef BZRREVNO
    #include "bzrrevno.h"
    #define DO_EXPAND(VAL)  VAL ## 0
    #define EXPAND(VAL)     DO_EXPAND(VAL)

    #if defined(BZR_REVISION) && (EXPAND(BZR_REVISION) != 0)
        const unsigned RevisionNumber = BZR_REVISION;
    #else
        const unsigned RevisionNumber = 0;
    #endif
#else
    #if REVISION_NUMBER
        const unsigned RevisionNumber = REVISION_NUMBER;
    #else
        const unsigned RevisionNumber = 0;
    #endif
#endif

#if BUILD_TYPE_ENUM
    const unsigned BuildType      = BUILD_TYPE_ENUM;
#else
    const unsigned BuildType      = BUILD_TYPE_UNKNOWN;
#endif
