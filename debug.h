/* debug.h -- Debug tracing infrastructure for es-shell */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdarg.h>

/* Debug categories - each can be enabled/disabled independently */
#define DEBUG_NONE     0x0000
#define DEBUG_PARSE    0x0001  /* Command parsing */
#define DEBUG_EVAL     0x0002  /* Expression evaluation */
#define DEBUG_PRIM     0x0004  /* Primitive dispatch */
#define DEBUG_FUNC     0x0008  /* Function calls */
#define DEBUG_LIST     0x0010  /* List operations */
#define DEBUG_TREE     0x0020  /* AST tree operations */
#define DEBUG_VAR      0x0040  /* Variable lookup */
#define DEBUG_ALL      0xFFFF

/* Compile-time debug control */
#ifdef ES_DEBUG
    extern int es_debug_flags;
    #define DEBUG_ENABLED(category) ((es_debug_flags & (category)) != 0)
    
    /* Debug output macros */
    #define DEBUG_TRACE(category, fmt, ...) \
        do { \
            if (DEBUG_ENABLED(category)) { \
                fprintf(stderr, "[%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                fflush(stderr); \
            } \
        } while(0)
    
    #define DEBUG_TRACE_ENTER(category, fmt, ...) \
        do { \
            if (DEBUG_ENABLED(category)) { \
                fprintf(stderr, "[%s:%d] ENTER: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                fflush(stderr); \
            } \
        } while(0)
    
    #define DEBUG_TRACE_EXIT(category, fmt, ...) \
        do { \
            if (DEBUG_ENABLED(category)) { \
                fprintf(stderr, "[%s:%d] EXIT: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                fflush(stderr); \
            } \
        } while(0)
    
    #define DEBUG_PRINT_LIST(category, list, prefix) \
        do { \
            if (DEBUG_ENABLED(category)) { \
                debug_print_list(list, prefix, __FUNCTION__, __LINE__); \
            } \
        } while(0)
    
    #define DEBUG_PRINT_TREE(category, tree, prefix) \
        do { \
            if (DEBUG_ENABLED(category)) { \
                debug_print_tree(tree, prefix, __FUNCTION__, __LINE__); \
            } \
        } while(0)
    
    /* Debug utility functions */
    extern void debug_print_list(List *list, const char *prefix, const char *func, int line);
    extern void debug_print_tree(Tree *tree, const char *prefix, const char *func, int line);
    extern void debug_init(void);
    extern void debug_set_flags(int flags);

#else
    /* No-op macros when debugging is disabled */
    #define DEBUG_TRACE(category, fmt, ...)
    #define DEBUG_TRACE_ENTER(category, fmt, ...)
    #define DEBUG_TRACE_EXIT(category, fmt, ...)
    #define DEBUG_PRINT_LIST(category, list, prefix)
    #define DEBUG_PRINT_TREE(category, tree, prefix)
    #define debug_init()
    #define debug_set_flags(flags)
#endif

#endif /* DEBUG_H */