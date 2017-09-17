#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#define MSWIN32
#else
    #ifdef __WINDOWS_
        #ifndef MSWIN32
        #define MSWIN32
        #endif // OS_WINDOWS
    #endif // __WINDOWS_
#endif


#ifdef __GNUC__
    #ifndef PACKED
        #define PACKED __attribute__ ((packed))
    #endif // PACKED
#endif // __GNUC__



typedef  unsigned short uint16_t;
typedef  unsigned char uint8_t;
typedef  unsigned int uint32_t;

#define TRUE 1
#define FALSE 0





#endif // TYPES_H_INCLUDED
