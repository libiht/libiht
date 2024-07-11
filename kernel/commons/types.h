#ifndef _COMMONS_TYPES_H
#define _COMMONS_TYPES_H

////////////////////////////////////////////////////////////////////////////////
//
//  File           : kernel/commons/types.h
//  Description    : This is the header file for the common types for the 
//                   libiht library.  
//                   
//
//   Author        : Thomason Zhao
//   Last Modified : July 10, 2024
//

// cpp cross compile handler
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//
// Type definitions

// Redefine types that works for both linux and windows platform
typedef signed      char        s8;
typedef unsigned    char        u8;
typedef signed      short       s16;
typedef unsigned    short       u16;
typedef signed      int         s32;
typedef unsigned    int         u32;
typedef signed      long long   s64;
typedef unsigned    long long   u64;

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // _COMMONS_TYPES_H