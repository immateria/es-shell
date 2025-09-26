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
#define DEBUG_ASSIGN   0x0080  /* Assignment operators */
#define DEBUG_TOKEN    0x0100  /* Tokenizer */
#define DEBUG_MEMORY   0x0200  /* Memory operations */
#define DEBUG_STACK    0x0400  /* Call stack tracing */
#define DEBUG_PERF     0x0800  /* Performance metrics */
#define DEBUG_PIPE     0x1000  /* Pipeline debugging */
#define DEBUG_BREAK    0x2000  /* Breakpoint system */
#define DEBUG_WATCH    0x4000  /* Variable watching */
#define DEBUG_INTER    0x8000  /* Interactive debugging */
#define DEBUG_INPUT    0x10000 /* Input handling */
#define DEBUG_ALL      0xFFFF

/* Interactive debugging modes */
typedef enum {
    DEBUG_MODE_OFF,        /* No interactive debugging */
    DEBUG_MODE_TRACE,      /* Trace execution but don't stop */
    DEBUG_MODE_STEP,       /* Stop at each statement */
    DEBUG_MODE_BREAK       /* Stop only at breakpoints */
} DebugMode;

/* Breakpoint types */
typedef enum {
    BREAK_LINE,           /* Break at specific line */
    BREAK_FUNCTION,       /* Break at function entry */
    BREAK_VAR_CHANGE,     /* Break when variable changes */
    BREAK_CONDITION,      /* Break when condition is true */
    BREAK_ERROR           /* Break on error */
} BreakType;

/* Breakpoint structure */
typedef struct Breakpoint {
    int id;
    BreakType type;
    char *location;       /* Function name, file:line, or variable name */
    char *condition;      /* Conditional expression (optional) */
    int enabled;
    int hit_count;
    struct Breakpoint *next;
} Breakpoint;

/* Performance measurement structure */
typedef struct PerfCounter {
    char *name;
    long start_time;
    long total_time;
    int call_count;
    struct PerfCounter *next;
} PerfCounter;

/* Call stack entry */
typedef struct CallFrame {
    char *function_name;
    char *location;
    long start_time;
    struct CallFrame *parent;
} CallFrame;

/* Debug event system structures */
typedef enum {
    DEBUG_EVENT_TRACE,        /* General debug trace event */
    DEBUG_EVENT_ENTER,        /* Function entry event */
    DEBUG_EVENT_EXIT,         /* Function exit event */
    DEBUG_EVENT_BREAKPOINT,   /* Breakpoint hit event */
    DEBUG_EVENT_VARIABLE,     /* Variable watch event */
    DEBUG_EVENT_PERFORMANCE,  /* Performance measurement event */
    DEBUG_EVENT_ERROR,        /* Error/exception event */
    DEBUG_EVENT_STACK         /* Call stack change event */
} DebugEventType;

/* Debug protocol output formats */
typedef enum {
    DEBUG_PROTOCOL_TEXT,      /* Traditional text output (default) */
    DEBUG_PROTOCOL_JSON,      /* Structured JSON output */
    DEBUG_PROTOCOL_DAP        /* Debug Adapter Protocol compatible */
} DebugProtocol;

/* Debug output levels */
typedef enum {
    DEBUG_LEVEL_INFO,
    DEBUG_LEVEL_WARN,
    DEBUG_LEVEL_ERROR,
    DEBUG_LEVEL_ENTER,
    DEBUG_LEVEL_EXIT
} DebugLevel;

/* Event data structure */
typedef struct DebugEvent {
    DebugEventType type;
    long timestamp;
    int category;
    DebugLevel level;
    const char *function;
    int line;
    const char *message;
    
    /* Event-specific data */
    union {
        struct {  /* For function entry/exit events */
            const char *function_name;
            const char *location;
            long duration_ms;  /* For exit events */
        } func_event;
        
        struct {  /* For variable watch events */
            const char *var_name;
            const char *old_value;
            const char *new_value;
        } var_event;
        
        struct {  /* For breakpoint events */
            int breakpoint_id;
            BreakType break_type;
            const char *condition;
            int hit_count;
        } break_event;
        
        struct {  /* For performance events */
            const char *counter_name;
            long duration_ms;
            int call_count;
            long total_ms;
        } perf_event;
        
        struct {  /* For call stack events */
            int stack_depth;
            CallFrame *current_frame;
        } stack_event;
    } data;
} DebugEvent;

/* Event handler function type */
typedef void (*DebugEventHandler)(const DebugEvent *event, void *userdata);

/* Event subscription structure */
typedef struct EventSubscription {
    DebugEventType event_type;
    int category_mask;            /* Which debug categories to listen for */
    DebugEventHandler handler;
    void *userdata;
    struct EventSubscription *next;
} EventSubscription;

/* Compile-time debug control */
#ifdef ES_DEBUG
    extern int es_debug_flags;
    extern DebugMode es_debug_mode;
    extern CallFrame *es_call_stack;
    extern Breakpoint *es_breakpoints;
    extern DebugProtocol es_debug_protocol;
    extern int debug_use_colors;
    extern FILE *debug_log_file;
    
    #define DEBUG_ENABLED(category) ((es_debug_flags & (category)) != 0)
    
    /* Enhanced debug output macros with timestamps and colors */
    #define DEBUG_TRACE(category, fmt, ...) \
        do { \
            if (DEBUG_ENABLED(category)) { \
                debug_output(category, DEBUG_LEVEL_INFO, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
            } \
        } while(0)
    
    #define DEBUG_TRACE_ENTER(category, fmt, ...) \
        do { \
            if (DEBUG_ENABLED(category)) { \
                debug_push_call(__FUNCTION__, __FILE__, __LINE__); \
                debug_output(category, DEBUG_LEVEL_ENTER, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
                if (debug_should_break(__FUNCTION__, BREAK_FUNCTION)) { \
                    debug_interactive_prompt(); \
                } \
            } \
        } while(0)
    
    #define DEBUG_TRACE_EXIT(category, fmt, ...) \
        do { \
            if (DEBUG_ENABLED(category)) { \
                debug_output(category, DEBUG_LEVEL_EXIT, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
                debug_pop_call(); \
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

    
    /* Performance measurement macros */
    #define DEBUG_PERF_START(name) \
        do { \
            if (DEBUG_ENABLED(DEBUG_PERF)) { \
                debug_perf_start(name); \
            } \
        } while(0)
    
    #define DEBUG_PERF_END(name) \
        do { \
            if (DEBUG_ENABLED(DEBUG_PERF)) { \
                debug_perf_end(name); \
            } \
        } while(0)
    
    /* Variable watching macros */
    #define DEBUG_VAR_WATCH(var_name, value) \
        do { \
            if (DEBUG_ENABLED(DEBUG_WATCH)) { \
                debug_var_watch(var_name, value, __FUNCTION__, __LINE__); \
            } \
        } while(0)
    
    /* Interactive debugging macros */
    #define DEBUG_CHECKPOINT() \
        do { \
            if (es_debug_mode == DEBUG_MODE_STEP) { \
                debug_interactive_prompt(); \
            } \
        } while(0)

    /* Debug utility functions */
    extern void debug_print_list(List *list, const char *prefix, const char *func, int line);
    extern void debug_print_tree(Tree *tree, const char *prefix, const char *func, int line);
    extern void debug_init(void);
    extern void debug_set_flags(int flags);
    extern void debug_set_mode(DebugMode mode);
    
    /* Enhanced debugging functions */
    extern void debug_output(int category, DebugLevel level, const char *func, int line, const char *fmt, ...);
    extern void debug_push_call(const char *func, const char *file, int line);
    extern void debug_pop_call(void);
    extern void debug_print_stack(void);
    extern int debug_should_break(const char *location, BreakType type);
    extern void debug_interactive_prompt(void);
    
    /* Breakpoint management */
    extern int debug_add_breakpoint(BreakType type, const char *location, const char *condition);
    extern void debug_remove_breakpoint(int id);
    extern void debug_list_breakpoints(void);
    extern void debug_enable_breakpoint(int id, int enabled);
    
    /* Performance measurement */
    extern void debug_perf_start(const char *name);
    extern void debug_perf_end(const char *name);
    extern void debug_perf_report(void);
    
    /* Variable watching */
    extern void debug_var_watch(const char *name, const char *value, const char *func, int line);
    extern void debug_add_watch(const char *var_name);
    extern void debug_remove_watch(const char *var_name);
    
    /* Event system functions */
    extern void debug_emit_event(const DebugEvent *event);
    extern int debug_subscribe(DebugEventType event_type, int category_mask, 
                              DebugEventHandler handler, void *userdata);
    extern void debug_unsubscribe(int subscription_id);
    extern void debug_set_protocol(DebugProtocol protocol);
    extern DebugProtocol debug_get_protocol(void);
    
    /* Protocol output functions */
    extern void debug_output_text(const DebugEvent *event);
    extern void debug_output_json(const DebugEvent *event);
    extern void debug_output_dap(const DebugEvent *event);
    
    /* Event creation helpers */
    extern DebugEvent debug_create_event(DebugEventType type, int category, DebugLevel level,
                                        const char *function, int line, const char *message);
    extern DebugEvent debug_create_enter_event(int category, const char *function, 
                                              const char *file, int line, const char *message);
    extern DebugEvent debug_create_exit_event(int category, const char *function, 
                                             const char *file, int line, const char *message, 
                                             long duration_ms);
    extern DebugEvent debug_create_variable_event(int category, const char *function, int line,
                                                 const char *var_name, const char *old_value, 
                                                 const char *new_value);
    extern DebugEvent debug_create_breakpoint_event(int category, const char *function, int line,
                                                   int breakpoint_id, BreakType break_type,
                                                   const char *condition, int hit_count);
    extern DebugEvent debug_create_performance_event(int category, const char *function, int line,
                                                    const char *counter_name, long duration_ms,
                                                    int call_count, long total_ms);
    extern DebugEvent debug_create_stack_event(int category, const char *function, int line,
                                              int stack_depth, CallFrame *current_frame);
    extern void debug_events_cleanup(void);

#else
    /* No-op macros when debugging is disabled */
    #define DEBUG_TRACE(category, fmt, ...)
    #define DEBUG_TRACE_ENTER(category, fmt, ...)
    #define DEBUG_TRACE_EXIT(category, fmt, ...)
    #define DEBUG_PRINT_LIST(category, list, prefix)
    #define DEBUG_PRINT_TREE(category, tree, prefix)
    #define DEBUG_PERF_START(name)
    #define DEBUG_PERF_END(name)
    #define DEBUG_VAR_WATCH(var_name, value)
    #define DEBUG_CHECKPOINT()
    #define debug_init()
    #define debug_set_flags(flags)
    #define debug_set_mode(mode)
#endif

#endif /* DEBUG_H */