#ifndef _COMMONS_TYPES_H
#define _COMMONS_TYPES_H

/* cpp cross compile handler */
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* Redefine types that works for both linux and windows platform */
typedef signed      char        s8;
typedef unsigned    char        u8;
typedef signed      short       s16;
typedef unsigned    short       u16;
typedef signed      int         s32;
typedef unsigned    int         u32;
typedef signed      long long   s64;
typedef unsigned    long long   u64;

#ifdef __cplusplus
}
#endif // __cplusplus


#endif /* _COMMONS_TYPES_H */