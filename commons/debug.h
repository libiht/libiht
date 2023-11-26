#ifndef _COMMONS_DEBUG_H
#define _COMMONS_DEBUG_H

/* cpp cross compile handler */
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* Debug control macros */
#define DEBUG_MSG
#define ON_INTELx

/* Dbg default log levels */
#define DBG_ERROR_LEVEL     (1 << 0)
#define DBG_ERROR_DESC      "ERROR"
#define DBG_WARNING_LEVEL   (1 << 1)
#define DBG_WARNING_DESC    "WARNING"
#define DBG_INFO_LEVEL      (1 << 2)
#define DBG_INFO_DESC       "INFO"
#define DBG_OUTPUT_LEVEL    (1 << 3)
#define DBG_OUTPUT_DESC     "OUTPUT"
#define DBG_MAX_LEVEL       (1 << 5)
#define DEFAULT_DBG_LEVEL   (DBG_ERROR_LEVEL | DBG_WARNING_LEVEL | DBG_INFO_LEVEL | DBG_OUTPUT_LEVEL)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* _COMMONS_DEBUG_H */