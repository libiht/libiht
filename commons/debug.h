#ifndef _COMMONS_DEBUG_H
#define _COMMONS_DEBUG_H

/* cpp cross compile handler */
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* Debug control macros */
#define DEBUG_MSG
#define ON_INTELx

void print_dbg(const char *format, ...);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* _COMMONS_DEBUG_H */