/* debug.c -- Debug utility functions for es-shell */

#include "es.h"
#include "debug.h"
#include "term.h"
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/* Forward declarations for static functions */
static long debug_get_timestamp(void);
static int debug_get_stack_depth(void);

#ifdef ES_DEBUG

/* Global debug state variables */
int es_debug_flags = DEBUG_NONE;
DebugMode es_debug_mode = DEBUG_MODE_OFF;
CallFrame *es_call_stack = NULL;
Breakpoint *es_breakpoints = NULL;
DebugProtocol es_debug_protocol = DEBUG_PROTOCOL_TEXT;

/* Static state for enhanced debugging */
static PerfCounter *perf_counters = NULL;
static int breakpoint_next_id = 1;
int debug_use_colors = 1;        /* Non-static for access by protocol handlers */
FILE *debug_log_file = NULL;     /* Non-static for access by protocol handlers */

/* Memory allocation - use malloc for debug structures to avoid GC complexity */

/* ANSI color codes for enhanced output */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m" 
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_GRAY    "\033[90m"

/* Initialize debug system */
void debug_init(void) {
    char *debug_env;
    fprintf(stderr, "DEBUG_INIT: Initializing debug system\n");
    
    /* Check environment variable for debug flags */
    debug_env = getenv("ES_DEBUG");
    fprintf(stderr, "DEBUG_INIT: ES_DEBUG env var = %s\n", debug_env ? debug_env : "(null)");
    
    /* Check for color output preference */
    debug_use_colors = (isatty(STDERR_FILENO) && getenv("NO_COLOR") == NULL);
    
    /* Check for debug log file */
    char *log_file = getenv("ES_DEBUG_LOG");
    if (log_file != NULL) {
        debug_log_file = fopen(log_file, "w");
        if (debug_log_file == NULL) {
            fprintf(stderr, "DEBUG_INIT: Failed to open debug log file: %s\n", log_file);
        }
    }
    
    /* Check for debug format preference */
    char *format_env = getenv("ES_DEBUG_FORMAT");
    if (format_env != NULL) {
        if (strcmp(format_env, "json") == 0) {
            es_debug_protocol = DEBUG_PROTOCOL_JSON;
            fprintf(stderr, "DEBUG_INIT: Using JSON output format\n");
        } else if (strcmp(format_env, "dap") == 0) {
            es_debug_protocol = DEBUG_PROTOCOL_DAP;
            fprintf(stderr, "DEBUG_INIT: Using DAP output format\n");
        } else if (strcmp(format_env, "text") == 0) {
            es_debug_protocol = DEBUG_PROTOCOL_TEXT;
            fprintf(stderr, "DEBUG_INIT: Using text output format (default)\n");
        } else {
            fprintf(stderr, "DEBUG_INIT: Unknown debug format '%s', using text\n", format_env);
            es_debug_protocol = DEBUG_PROTOCOL_TEXT;
        }
    }
    
    if (debug_env != NULL) {
        /* Parse comma-separated debug categories */
        char *env_copy = strdup(debug_env);
        char *token = strtok(env_copy, ",");
        
        while (token != NULL) {
            if (strcmp(token, "all") == 0) {
                es_debug_flags = DEBUG_ALL;
            } else if (strcmp(token, "parse") == 0) {
                es_debug_flags |= DEBUG_PARSE;
            } else if (strcmp(token, "eval") == 0) {
                es_debug_flags |= DEBUG_EVAL;
            } else if (strcmp(token, "prim") == 0) {
                es_debug_flags |= DEBUG_PRIM;
            } else if (strcmp(token, "func") == 0) {
                es_debug_flags |= DEBUG_FUNC;
            } else if (strcmp(token, "list") == 0) {
                es_debug_flags |= DEBUG_LIST;
            } else if (strcmp(token, "tree") == 0) {
                es_debug_flags |= DEBUG_TREE;
            } else if (strcmp(token, "var") == 0) {
                es_debug_flags |= DEBUG_VAR;
            } else if (strcmp(token, "assign") == 0) {
                es_debug_flags |= DEBUG_ASSIGN;
            } else if (strcmp(token, "token") == 0) {
                es_debug_flags |= DEBUG_TOKEN;
            } else if (strcmp(token, "memory") == 0) {
                es_debug_flags |= DEBUG_MEMORY;
            } else if (strcmp(token, "stack") == 0) {
                es_debug_flags |= DEBUG_STACK;
            } else if (strcmp(token, "perf") == 0) {
                es_debug_flags |= DEBUG_PERF;
            } else if (strcmp(token, "pipe") == 0) {
                es_debug_flags |= DEBUG_PIPE;
            } else if (strcmp(token, "break") == 0) {
                es_debug_flags |= DEBUG_BREAK;
            } else if (strcmp(token, "watch") == 0) {
                es_debug_flags |= DEBUG_WATCH;
            } else if (strcmp(token, "inter") == 0) {
                es_debug_flags |= DEBUG_INTER;
                es_debug_mode = DEBUG_MODE_BREAK; /* Enable interactive mode */
            } else if (strcmp(token, "step") == 0) {
                es_debug_flags |= DEBUG_INTER;
                es_debug_mode = DEBUG_MODE_STEP; /* Enable step mode */
            } else {
                /* Try to parse as numeric flags */
                es_debug_flags |= atoi(token);
            }
            token = strtok(NULL, ",");
        }
        free(env_copy);
    }
    
    if (es_debug_flags != DEBUG_NONE) {
        fprintf(stderr, "DEBUG: es-shell debug tracing enabled (flags=0x%04x)\n", es_debug_flags);
    }
}

/* Set debug flags programmatically */
void debug_set_flags(int flags) {
    es_debug_flags = flags;
}

/* Print a list structure for debugging */
void debug_print_list(List *list, const char *prefix, const char *func, int line) {
    List *lp;
    int count = 0;
    
    fprintf(stderr, "[%s:%d] %s List contents:\n", func, line, prefix);
    
    if (list == NULL) {
        fprintf(stderr, "[%s:%d]   (null list)\n", func, line);
        return;
    }
    
    for (lp = list; lp != NULL; lp = lp->next) {
        fprintf(stderr, "[%s:%d]   [%d]: ", func, line, count);
        
        if (lp->term == NULL) {
            fprintf(stderr, "(null term)");
        } else if (lp->term->str == NULL) {
            fprintf(stderr, "(null string)");
        } else {
            fprintf(stderr, "\"%s\"", lp->term->str);
        }
        
        if (lp->next != NULL) {
            fprintf(stderr, " ->");
        }
        fprintf(stderr, "\n");
        count++;
    }
    
    fprintf(stderr, "[%s:%d]   Total: %d items\n", func, line, count);
    fflush(stderr);
}

/* Print a tree structure for debugging */  
void debug_print_tree(Tree *tree, const char *prefix, const char *func, int line) {
    fprintf(stderr, "[%s:%d] %s Tree:\n", func, line, prefix);
    
    if (tree == NULL) {
        fprintf(stderr, "[%s:%d]   (null tree)\n", func, line);
        return;
    }
    
    fprintf(stderr, "[%s:%d]   kind: %d\n", func, line, tree->kind);
    
    switch (tree->kind) {
        case nWord:
            fprintf(stderr, "[%s:%d]   word: \"%s\"\n", func, line, 
                   tree->u[0].s ? tree->u[0].s : "(null)");
            break;
            
        case nQword:
            fprintf(stderr, "[%s:%d]   qword: \"%s\"\n", func, line,
                   tree->u[0].s ? tree->u[0].s : "(null)");
            break;
            
        case nPrim:
            fprintf(stderr, "[%s:%d]   primitive: \"%s\"\n", func, line,
                   tree->u[0].s ? tree->u[0].s : "(null)");
            break;
            
        case nCall:
            fprintf(stderr, "[%s:%d]   call:\n", func, line);
            if (tree->u[0].p) {
                debug_print_tree(tree->u[0].p, "    function", func, line);
            }
            if (tree->u[1].p) {
                debug_print_tree(tree->u[1].p, "    args", func, line);
            }
            break;
            
        case nThunk:
            fprintf(stderr, "[%s:%d]   thunk:\n", func, line);
            if (tree->u[0].p) {
                debug_print_tree(tree->u[0].p, "    body", func, line);
            }
            break;
            
        case nList:
            fprintf(stderr, "[%s:%d]   list:\n", func, line);
            if (tree->u[0].p) {
                debug_print_tree(tree->u[0].p, "    car", func, line);
            }
            if (tree->u[1].p) {
                debug_print_tree(tree->u[1].p, "    cdr", func, line);
            }
            break;
            
        default:
            fprintf(stderr, "[%s:%d]   (unknown tree kind %d)\n", func, line, tree->kind);
            break;
    }
    
    fflush(stderr);
}

/* Get current timestamp in milliseconds */
static long debug_get_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

/* Enhanced debug output with colors and formatting */
void debug_output(int category, DebugLevel level, const char *func, int line, const char *fmt, ...) {
    va_list args;
    const char *color = "";
    const char *prefix = "";
    long timestamp = debug_get_timestamp();
    char message_buffer[1024];
    
    /* Format the message */
    va_start(args, fmt);
    vsnprintf(message_buffer, sizeof(message_buffer), fmt, args);
    va_end(args);
    
    /* Create and emit debug event (only for non-TEXT protocols) */
    if (es_debug_protocol != DEBUG_PROTOCOL_TEXT) {
        DebugEvent event = debug_create_event(DEBUG_EVENT_TRACE, category, level, func, line, message_buffer);
        debug_emit_event(&event);
        return;  /* Event emission handles output for structured protocols */
    }
    
    /* Traditional text output for backward compatibility */
    /* Choose color and prefix based on level */
    if (debug_use_colors) {
        switch (level) {
            case DEBUG_LEVEL_ERROR: color = COLOR_RED; prefix = "ERROR"; break;
            case DEBUG_LEVEL_WARN:  color = COLOR_YELLOW; prefix = "WARN"; break;
            case DEBUG_LEVEL_ENTER: color = COLOR_GREEN; prefix = "ENTER"; break;
            case DEBUG_LEVEL_EXIT:  color = COLOR_BLUE; prefix = "EXIT"; break;
            default:                color = COLOR_GRAY; prefix = "INFO"; break;
        }
    }
    
    /* Output to stderr */
    fprintf(stderr, "%s[%ld][%s:%d][%s] %s%s\n", 
           color, timestamp, func, line, prefix, message_buffer,
           debug_use_colors ? COLOR_RESET : "");
    
    /* Also output to log file if configured */
    if (debug_log_file != NULL) {
        fprintf(debug_log_file, "[%ld][%s:%d][%s] %s\n", 
               timestamp, func, line, prefix, message_buffer);
        fflush(debug_log_file);
    }
    
    fflush(stderr);
}

/* Call stack management */
void debug_push_call(const char *func, const char *file, int line) {
    if (!DEBUG_ENABLED(DEBUG_STACK)) return;
    
    CallFrame *frame = malloc(sizeof(CallFrame));
    frame->function_name = strdup(func);
    
    /* Allocate and format location string */
    size_t loc_len = strlen(file) + 20;  /* file + ":line" + margin */
    frame->location = malloc(loc_len);
    snprintf(frame->location, loc_len, "%s:%d", file, line);
    frame->start_time = debug_get_timestamp();
    frame->parent = es_call_stack;
    es_call_stack = frame;
    
    /* Emit stack change event */
    if (es_debug_protocol != DEBUG_PROTOCOL_TEXT) {
        DebugEvent event = debug_create_stack_event(DEBUG_STACK, func, line, 
                                                   debug_get_stack_depth(), frame);
        debug_emit_event(&event);
    }
    
    debug_output(DEBUG_STACK, DEBUG_LEVEL_INFO, func, line, "PUSH: Stack depth now %d", 
                debug_get_stack_depth());
}

void debug_pop_call(void) {
    if (!DEBUG_ENABLED(DEBUG_STACK) || es_call_stack == NULL) return;
    
    CallFrame *frame = es_call_stack;
    long duration = debug_get_timestamp() - frame->start_time;
    
    /* Emit stack change event before popping */
    if (es_debug_protocol != DEBUG_PROTOCOL_TEXT) {
        DebugEvent event = debug_create_stack_event(DEBUG_STACK, frame->function_name, 0,
                                                   debug_get_stack_depth() - 1, frame->parent);
        debug_emit_event(&event);
    }
    
    debug_output(DEBUG_STACK, DEBUG_LEVEL_INFO, frame->function_name, 0, 
                "POP: Duration %ldms, Stack depth now %d", 
                duration, debug_get_stack_depth() - 1);
    
    es_call_stack = frame->parent;
    free(frame->function_name);
    free(frame->location);
    free(frame);
}

/* Get current call stack depth */
static int debug_get_stack_depth(void) {
    int depth = 0;
    CallFrame *frame = es_call_stack;
    while (frame != NULL) {
        depth++;
        frame = frame->parent;
    }
    return depth;
}

/* Print current call stack */
void debug_print_stack(void) {
    CallFrame *frame = es_call_stack;
    int depth = 0;
    
    fprintf(stderr, "\n=== Call Stack ===\n");
    while (frame != NULL) {
        long duration = debug_get_timestamp() - frame->start_time;
        fprintf(stderr, "#%d  %s() at %s [%ldms]\n", 
               depth, frame->function_name, frame->location, duration);
        frame = frame->parent;
        depth++;
    }
    fprintf(stderr, "==================\n\n");
    fflush(stderr);
}

/* Breakpoint management */
int debug_add_breakpoint(BreakType type, const char *location, const char *condition) {
    Breakpoint *bp = malloc(sizeof(Breakpoint));
    bp->id = breakpoint_next_id++;
    bp->type = type;
    bp->location = strdup(location);
    bp->condition = condition ? strdup(condition) : NULL;
    bp->enabled = 1;
    bp->hit_count = 0;
    bp->next = es_breakpoints;
    es_breakpoints = bp;
    
    debug_output(DEBUG_BREAK, DEBUG_LEVEL_INFO, __FUNCTION__, __LINE__,
                "Added breakpoint #%d at %s", bp->id, location);
    return bp->id;
}

void debug_remove_breakpoint(int id) {
    Breakpoint **bp = &es_breakpoints;
    while (*bp != NULL) {
        if ((*bp)->id == id) {
            Breakpoint *to_remove = *bp;
            *bp = to_remove->next;
            debug_output(DEBUG_BREAK, DEBUG_LEVEL_INFO, __FUNCTION__, __LINE__,
                        "Removed breakpoint #%d", id);
            free(to_remove->location);
            if (to_remove->condition) free(to_remove->condition);
            free(to_remove);
            return;
        }
        bp = &(*bp)->next;
    }
}

void debug_list_breakpoints(void) {
    Breakpoint *bp = es_breakpoints;
    fprintf(stderr, "\n=== Breakpoints ===\n");
    if (bp == NULL) {
        fprintf(stderr, "No breakpoints set.\n");
    } else {
        while (bp != NULL) {
            fprintf(stderr, "#%d %s %s at %s", 
                   bp->id, bp->enabled ? "ENABLED" : "DISABLED",
                   bp->type == BREAK_FUNCTION ? "FUNCTION" : 
                   bp->type == BREAK_VAR_CHANGE ? "WATCH" : "LINE",
                   bp->location);
            if (bp->condition) {
                fprintf(stderr, " [condition: %s]", bp->condition);
            }
            fprintf(stderr, " [hits: %d]\n", bp->hit_count);
            bp = bp->next;
        }
    }
    fprintf(stderr, "==================\n\n");
    fflush(stderr);
}

/* Check if execution should break at current location */
int debug_should_break(const char *location, BreakType type) {
    if (!DEBUG_ENABLED(DEBUG_BREAK)) return 0;
    
    Breakpoint *bp = es_breakpoints;
    while (bp != NULL) {
        if (bp->enabled && bp->type == type) {
            if (strcmp(bp->location, location) == 0) {
                bp->hit_count++;
                debug_output(DEBUG_BREAK, DEBUG_LEVEL_INFO, __FUNCTION__, __LINE__,
                            "Hit breakpoint #%d at %s (hit count: %d)", 
                            bp->id, location, bp->hit_count);
                return 1;
            }
        }
        bp = bp->next;
    }
    return 0;
}

/* Interactive debugging prompt */
void debug_interactive_prompt(void) {
    if (!DEBUG_ENABLED(DEBUG_INTER)) return;
    
    char input[256];
    
    fprintf(stderr, "\n%s=== ES DEBUGGER ===%s\n", 
           debug_use_colors ? COLOR_CYAN : "", 
           debug_use_colors ? COLOR_RESET : "");
    fprintf(stderr, "Commands: (c)ontinue, (s)tep, (bt) backtrace, (l)ist breakpoints, (q)uit, (h)elp\n");
    
    while (1) {
        fprintf(stderr, "(es-debug) ");
        fflush(stderr);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        /* Remove newline */
        char *newline = strchr(input, '\n');
        if (newline) *newline = '\0';
        
        if (strlen(input) == 0 || strcmp(input, "c") == 0 || strcmp(input, "continue") == 0) {
            break;
        } else if (strcmp(input, "s") == 0 || strcmp(input, "step") == 0) {
            es_debug_mode = DEBUG_MODE_STEP;
            break;
        } else if (strcmp(input, "bt") == 0 || strcmp(input, "backtrace") == 0) {
            debug_print_stack();
        } else if (strcmp(input, "l") == 0 || strcmp(input, "list") == 0) {
            debug_list_breakpoints();
        } else if (strcmp(input, "perf") == 0) {
            debug_perf_report();
        } else if (strcmp(input, "q") == 0 || strcmp(input, "quit") == 0) {
            exit(0);
        } else if (strcmp(input, "h") == 0 || strcmp(input, "help") == 0) {
            fprintf(stderr, "Available commands:\n");
            fprintf(stderr, "  c, continue  - Continue execution\n");
            fprintf(stderr, "  s, step      - Step to next statement\n");
            fprintf(stderr, "  bt, backtrace - Show call stack\n");
            fprintf(stderr, "  l, list      - List breakpoints\n");
            fprintf(stderr, "  perf         - Show performance report\n");
            fprintf(stderr, "  q, quit      - Quit debugger\n");
            fprintf(stderr, "  h, help      - Show this help\n");
        } else {
            fprintf(stderr, "Unknown command: %s (type 'h' for help)\n", input);
        }
    }
}

/* Performance measurement */
void debug_perf_start(const char *name) {
    PerfCounter *counter = perf_counters;
    while (counter != NULL) {
        if (strcmp(counter->name, name) == 0) {
            counter->start_time = debug_get_timestamp();
            return;
        }
        counter = counter->next;
    }
    
    /* Create new counter */
    counter = malloc(sizeof(PerfCounter));
    counter->name = strdup(name);
    counter->start_time = debug_get_timestamp();
    counter->total_time = 0;
    counter->call_count = 0;
    counter->next = perf_counters;
    perf_counters = counter;
}

void debug_perf_end(const char *name) {
    PerfCounter *counter = perf_counters;
    long end_time = debug_get_timestamp();
    
    while (counter != NULL) {
        if (strcmp(counter->name, name) == 0) {
            counter->total_time += (end_time - counter->start_time);
            counter->call_count++;
            return;
        }
        counter = counter->next;
    }
}

void debug_perf_report(void) {
    PerfCounter *counter = perf_counters;
    
    fprintf(stderr, "\n=== Performance Report ===\n");
    fprintf(stderr, "%-20s %8s %10s %8s\n", "Function", "Calls", "Total(ms)", "Avg(ms)");
    fprintf(stderr, "%-20s %8s %10s %8s\n", "--------", "-----", "---------", "-------");
    
    while (counter != NULL) {
        long avg_time = counter->call_count > 0 ? counter->total_time / counter->call_count : 0;
        fprintf(stderr, "%-20s %8d %10ld %8ld\n", 
               counter->name, counter->call_count, counter->total_time, avg_time);
        counter = counter->next;
    }
    fprintf(stderr, "=========================\n\n");
    fflush(stderr);
}

/* Variable watching */
void debug_var_watch(const char *name, const char *value, const char *func, int line) {
    debug_output(DEBUG_WATCH, DEBUG_LEVEL_INFO, func, line,
                "WATCH: %s = \"%s\"", name, value ? value : "(null)");
}

/* Set debug mode */
void debug_set_mode(DebugMode mode) {
    es_debug_mode = mode;
    debug_output(DEBUG_INTER, DEBUG_LEVEL_INFO, __FUNCTION__, __LINE__,
                "Debug mode changed to %s", 
                mode == DEBUG_MODE_OFF ? "OFF" :
                mode == DEBUG_MODE_TRACE ? "TRACE" :
                mode == DEBUG_MODE_STEP ? "STEP" : "BREAK");
}

#endif /* ES_DEBUG */