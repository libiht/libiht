#ifndef _COMMONS_DEBUG_H
#define _COMMONS_DEBUG_H

/* Debug control macros */
#define DEBUG_MSG
#define ON_INTELx

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void print_dbg(const char *format, ...);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* _COMMONS_DEBUG_H */