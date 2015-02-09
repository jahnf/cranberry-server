#ifndef ___STDINT_H_
#define ___STDINT_H_

#ifdef _WIN32
    /* win64 is llp64 so these are the same for 32/64bit
     * so no check for _WIN64 is required. */
    typedef unsigned char uint8_t;
    typedef signed char int8_t;
    typedef unsigned short uint16_t;
    typedef signed short int16_t;
    typedef unsigned int uint32_t;
    typedef signed int int32_t;
    typedef unsigned __int64 uint64_t;
    typedef signed __int64 int64_t;
#else
    #include <stdint.h>
#endif

#endif /* ___STDINT_H_ */
