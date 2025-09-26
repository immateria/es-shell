/* debug_protocol.c -- Debug protocol output formatters for es-shell */

#include "es.h"
#include "debug.h"
#include <stdio.h>
#include <time.h>

#ifdef ES_DEBUG

/* Forward declarations */
static const char* debug_event_type_to_string(DebugEventType type);
static const char* debug_level_to_string(DebugLevel level);
static const char* debug_category_to_string(int category);
static void debug_escape_json_string(const char *str, char *buffer, size_t buffer_size);

/* Text protocol output (backward compatible) */
void debug_output_text(const DebugEvent *event) {
    if (event == NULL) return;
    
    /* For text protocol, we delegate back to the original debug_output logic.
     * This maintains 100% backward compatibility. */
     
    /* This function is primarily used when events are emitted programmatically
     * but we want text output. The main debug_output() function handles most cases. */
    
    extern int debug_use_colors;
    extern FILE *debug_log_file;
    
    const char *color = "";
    const char *prefix = "";
    
    /* Choose color and prefix based on level */
    if (debug_use_colors) {
        switch (event->level) {
            case DEBUG_LEVEL_ERROR: color = "\033[31m"; prefix = "ERROR"; break;
            case DEBUG_LEVEL_WARN:  color = "\033[33m"; prefix = "WARN"; break;
            case DEBUG_LEVEL_ENTER: color = "\033[32m"; prefix = "ENTER"; break;
            case DEBUG_LEVEL_EXIT:  color = "\033[34m"; prefix = "EXIT"; break;
            default:                color = "\033[90m"; prefix = "INFO"; break;
        }
    }
    
    /* Output to stderr */
    fprintf(stderr, "%s[%ld][%s:%d][%s] %s%s\n", 
           color, event->timestamp, event->function ? event->function : "", 
           event->line, prefix, event->message ? event->message : "",
           debug_use_colors ? "\033[0m" : "");
    
    /* Also output to log file if configured */
    if (debug_log_file != NULL) {
        fprintf(debug_log_file, "[%ld][%s:%d][%s] %s\n", 
               event->timestamp, event->function ? event->function : "",
               event->line, prefix, event->message ? event->message : "");
        fflush(debug_log_file);
    }
    
    fflush(stderr);
}

/* JSON protocol output */
void debug_output_json(const DebugEvent *event) {
    if (event == NULL) return;
    
    fprintf(stderr, "{\n");
    fprintf(stderr, "  \"type\": \"debug_event\",\n");
    fprintf(stderr, "  \"timestamp\": %ld,\n", event->timestamp);
    fprintf(stderr, "  \"event_type\": \"%s\",\n", debug_event_type_to_string(event->type));
    fprintf(stderr, "  \"category\": \"%s\",\n", debug_category_to_string(event->category));
    fprintf(stderr, "  \"level\": \"%s\",\n", debug_level_to_string(event->level));
    fprintf(stderr, "  \"function\": \"%s\",\n", event->function ? event->function : "");
    fprintf(stderr, "  \"line\": %d,\n", event->line);
    
    /* Escape and output message */
    if (event->message) {
        char escaped_message[1024];
        debug_escape_json_string(event->message, escaped_message, sizeof(escaped_message));
        fprintf(stderr, "  \"message\": \"%s\",\n", escaped_message);
    } else {
        fprintf(stderr, "  \"message\": null,\n");
    }
    
    /* Output event-specific data */
    fprintf(stderr, "  \"data\": {\n");
    
    switch (event->type) {
        case DEBUG_EVENT_ENTER:
        case DEBUG_EVENT_EXIT:
            fprintf(stderr, "    \"function_name\": \"%s\",\n", 
                   event->data.func_event.function_name ? event->data.func_event.function_name : "");
            fprintf(stderr, "    \"location\": \"%s\"", 
                   event->data.func_event.location ? event->data.func_event.location : "");
            if (event->type == DEBUG_EVENT_EXIT) {
                fprintf(stderr, ",\n    \"duration_ms\": %ld", event->data.func_event.duration_ms);
            }
            fprintf(stderr, "\n");
            break;
            
        case DEBUG_EVENT_VARIABLE:
            fprintf(stderr, "    \"variable_name\": \"%s\",\n", 
                   event->data.var_event.var_name ? event->data.var_event.var_name : "");
            fprintf(stderr, "    \"old_value\": \"%s\",\n", 
                   event->data.var_event.old_value ? event->data.var_event.old_value : "");
            fprintf(stderr, "    \"new_value\": \"%s\"\n", 
                   event->data.var_event.new_value ? event->data.var_event.new_value : "");
            break;
            
        case DEBUG_EVENT_BREAKPOINT:
            fprintf(stderr, "    \"breakpoint_id\": %d,\n", event->data.break_event.breakpoint_id);
            fprintf(stderr, "    \"break_type\": %d,\n", event->data.break_event.break_type);
            fprintf(stderr, "    \"condition\": \"%s\",\n", 
                   event->data.break_event.condition ? event->data.break_event.condition : "");
            fprintf(stderr, "    \"hit_count\": %d\n", event->data.break_event.hit_count);
            break;
            
        case DEBUG_EVENT_PERFORMANCE:
            fprintf(stderr, "    \"counter_name\": \"%s\",\n", 
                   event->data.perf_event.counter_name ? event->data.perf_event.counter_name : "");
            fprintf(stderr, "    \"duration_ms\": %ld,\n", event->data.perf_event.duration_ms);
            fprintf(stderr, "    \"call_count\": %d,\n", event->data.perf_event.call_count);
            fprintf(stderr, "    \"total_ms\": %ld\n", event->data.perf_event.total_ms);
            break;
            
        case DEBUG_EVENT_STACK:
            fprintf(stderr, "    \"stack_depth\": %d\n", event->data.stack_event.stack_depth);
            /* Note: We don't serialize the CallFrame pointer for security reasons */
            break;
            
        default:
            fprintf(stderr, "    \"raw_data\": \"event_type_%d\"", event->type);
            break;
    }
    
    fprintf(stderr, "  }\n");
    fprintf(stderr, "}\n");
    fflush(stderr);
}

/* DAP (Debug Adapter Protocol) compatible output */
void debug_output_dap(const DebugEvent *event) {
    if (event == NULL) return;
    
    /* DAP uses JSON-RPC 2.0 format with specific message types */
    fprintf(stderr, "{\n");
    fprintf(stderr, "  \"seq\": %ld,\n", event->timestamp);  /* Use timestamp as sequence */
    fprintf(stderr, "  \"type\": \"event\",\n");
    
    /* Map ES debug events to DAP event types */
    switch (event->type) {
        case DEBUG_EVENT_TRACE:
            fprintf(stderr, "  \"event\": \"output\",\n");
            fprintf(stderr, "  \"body\": {\n");
            fprintf(stderr, "    \"category\": \"stdout\",\n");
            fprintf(stderr, "    \"output\": \"%s\\n\"\n", event->message ? event->message : "");
            fprintf(stderr, "  }\n");
            break;
            
        case DEBUG_EVENT_BREAKPOINT:
            fprintf(stderr, "  \"event\": \"stopped\",\n");
            fprintf(stderr, "  \"body\": {\n");
            fprintf(stderr, "    \"reason\": \"breakpoint\",\n");
            fprintf(stderr, "    \"description\": \"Paused on breakpoint\",\n");
            fprintf(stderr, "    \"threadId\": 1,\n");
            fprintf(stderr, "    \"hitBreakpointIds\": [%d]\n", event->data.break_event.breakpoint_id);
            fprintf(stderr, "  }\n");
            break;
            
        case DEBUG_EVENT_VARIABLE:
            /* DAP doesn't have a direct variable change event, so we use output */
            fprintf(stderr, "  \"event\": \"output\",\n");
            fprintf(stderr, "  \"body\": {\n");
            fprintf(stderr, "    \"category\": \"console\",\n");
            fprintf(stderr, "    \"output\": \"Variable %s changed: %s -> %s\\n\"\n", 
                   event->data.var_event.var_name ? event->data.var_event.var_name : "",
                   event->data.var_event.old_value ? event->data.var_event.old_value : "",
                   event->data.var_event.new_value ? event->data.var_event.new_value : "");
            fprintf(stderr, "  }\n");
            break;
            
        case DEBUG_EVENT_ENTER:
        case DEBUG_EVENT_EXIT:
            /* Function calls as output events */
            fprintf(stderr, "  \"event\": \"output\",\n");
            fprintf(stderr, "  \"body\": {\n");
            fprintf(stderr, "    \"category\": \"stdout\",\n");
            fprintf(stderr, "    \"output\": \"%s %s() at %s\\n\"\n", 
                   (event->type == DEBUG_EVENT_ENTER) ? "ENTER" : "EXIT",
                   event->data.func_event.function_name ? event->data.func_event.function_name : "",
                   event->data.func_event.location ? event->data.func_event.location : "");
            fprintf(stderr, "  }\n");
            break;
            
        default:
            /* Generic output for other event types */
            fprintf(stderr, "  \"event\": \"output\",\n");
            fprintf(stderr, "  \"body\": {\n");
            fprintf(stderr, "    \"category\": \"stdout\",\n");
            fprintf(stderr, "    \"output\": \"[%s] %s\\n\"\n", 
                   debug_event_type_to_string(event->type),
                   event->message ? event->message : "");
            fprintf(stderr, "  }\n");
            break;
    }
    
    fprintf(stderr, "}\n");
    fflush(stderr);
}

/* Helper functions */
static const char* debug_event_type_to_string(DebugEventType type) {
    switch (type) {
        case DEBUG_EVENT_TRACE:       return "trace";
        case DEBUG_EVENT_ENTER:       return "enter";
        case DEBUG_EVENT_EXIT:        return "exit";
        case DEBUG_EVENT_BREAKPOINT:  return "breakpoint";
        case DEBUG_EVENT_VARIABLE:    return "variable";
        case DEBUG_EVENT_PERFORMANCE: return "performance";
        case DEBUG_EVENT_ERROR:       return "error";
        case DEBUG_EVENT_STACK:       return "stack";
        default:                      return "unknown";
    }
}

static const char* debug_level_to_string(DebugLevel level) {
    switch (level) {
        case DEBUG_LEVEL_INFO:  return "info";
        case DEBUG_LEVEL_WARN:  return "warn";
        case DEBUG_LEVEL_ERROR: return "error";
        case DEBUG_LEVEL_ENTER: return "enter";
        case DEBUG_LEVEL_EXIT:  return "exit";
        default:                return "unknown";
    }
}

static const char* debug_category_to_string(int category) {
    /* Return the primary category name (first bit set) */
    if (category & DEBUG_PARSE)  return "parse";
    if (category & DEBUG_EVAL)   return "eval";
    if (category & DEBUG_PRIM)   return "prim";
    if (category & DEBUG_FUNC)   return "func";
    if (category & DEBUG_LIST)   return "list";
    if (category & DEBUG_TREE)   return "tree";
    if (category & DEBUG_VAR)    return "var";
    if (category & DEBUG_ASSIGN) return "assign";
    if (category & DEBUG_TOKEN)  return "token";
    if (category & DEBUG_MEMORY) return "memory";
    if (category & DEBUG_STACK)  return "stack";
    if (category & DEBUG_PERF)   return "perf";
    if (category & DEBUG_PIPE)   return "pipe";
    if (category & DEBUG_BREAK)  return "break";
    if (category & DEBUG_WATCH)  return "watch";
    if (category & DEBUG_INTER)  return "inter";
    return "none";
}

static void debug_escape_json_string(const char *str, char *buffer, size_t buffer_size) {
    if (str == NULL || buffer == NULL || buffer_size < 2) {
        if (buffer && buffer_size > 0) buffer[0] = '\0';
        return;
    }
    
    size_t src_len = strlen(str);
    size_t dst_pos = 0;
    
    for (size_t src_pos = 0; src_pos < src_len && dst_pos < buffer_size - 2; src_pos++) {
        char c = str[src_pos];
        
        /* Escape special JSON characters */
        switch (c) {
            case '"':
                if (dst_pos < buffer_size - 3) {
                    buffer[dst_pos++] = '\\';
                    buffer[dst_pos++] = '"';
                }
                break;
            case '\\':
                if (dst_pos < buffer_size - 3) {
                    buffer[dst_pos++] = '\\';
                    buffer[dst_pos++] = '\\';
                }
                break;
            case '\n':
                if (dst_pos < buffer_size - 3) {
                    buffer[dst_pos++] = '\\';
                    buffer[dst_pos++] = 'n';
                }
                break;
            case '\r':
                if (dst_pos < buffer_size - 3) {
                    buffer[dst_pos++] = '\\';
                    buffer[dst_pos++] = 'r';
                }
                break;
            case '\t':
                if (dst_pos < buffer_size - 3) {
                    buffer[dst_pos++] = '\\';
                    buffer[dst_pos++] = 't';
                }
                break;
            default:
                /* Control characters need to be escaped as \uXXXX */
                if (c < 32 || c > 126) {
                    if (dst_pos < buffer_size - 7) {
                        snprintf(&buffer[dst_pos], 7, "\\u%04x", (unsigned char)c);
                        dst_pos += 6;
                    }
                } else {
                    buffer[dst_pos++] = c;
                }
                break;
        }
    }
    
    buffer[dst_pos] = '\0';
}

#endif /* ES_DEBUG */