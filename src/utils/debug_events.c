/* debug_events.c -- Debug event system for es-shell */

#include "es.h"
#include "debug.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef ES_DEBUG

/* Global event system state */
static EventSubscription *event_subscriptions = NULL;
static int next_subscription_id = 1;

/* Forward declarations */
static long debug_event_get_timestamp(void);
static void debug_free_subscription(EventSubscription *sub);

/* Get current timestamp in milliseconds */
static long debug_event_get_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

/* Emit a debug event to all registered handlers */
void debug_emit_event(const DebugEvent *event) {
    if (event == NULL) return;
    
    EventSubscription *sub = event_subscriptions;
    while (sub != NULL) {
        /* Check if this subscription matches the event */
        bool type_matches = (sub->event_type == event->type);
        bool category_matches = (sub->category_mask == 0 || 
                               (sub->category_mask & event->category) != 0);
        
        if (type_matches && category_matches) {
            /* Call the handler */
            sub->handler(event, sub->userdata);
        }
        sub = sub->next;
    }
    
    /* Always output via the configured protocol */
    switch (es_debug_protocol) {
        case DEBUG_PROTOCOL_TEXT:
            debug_output_text(event);
            break;
        case DEBUG_PROTOCOL_JSON:
            debug_output_json(event);
            break;
        case DEBUG_PROTOCOL_DAP:
            debug_output_dap(event);
            break;
    }
}

/* Subscribe to debug events */
int debug_subscribe(DebugEventType event_type, int category_mask,
                   DebugEventHandler handler, void *userdata) {
    if (handler == NULL) return -1;
    
    EventSubscription *sub = malloc(sizeof(EventSubscription));
    if (sub == NULL) return -1;
    
    sub->event_type = event_type;
    sub->category_mask = category_mask;
    sub->handler = handler;
    sub->userdata = userdata;
    sub->next = event_subscriptions;
    
    /* Assign unique ID */
    int id = next_subscription_id++;
    
    /* Insert at head of list */
    event_subscriptions = sub;
    
    return id;
}

/* Unsubscribe from debug events */
void debug_unsubscribe(int subscription_id) {
    /* Note: For simplicity, we're not implementing ID tracking in this version.
     * In a production system, we'd maintain an ID->subscription mapping.
     * For now, this is a placeholder for the interface. */
}

/* Set debug protocol format */
void debug_set_protocol(DebugProtocol protocol) {
    es_debug_protocol = protocol;
}

/* Get current debug protocol format */
DebugProtocol debug_get_protocol(void) {
    return es_debug_protocol;
}

/* Helper function to create debug events */
DebugEvent debug_create_event(DebugEventType type, int category, DebugLevel level,
                             const char *function, int line, const char *message) {
    DebugEvent event = {0};
    event.type = type;
    event.timestamp = debug_event_get_timestamp();
    event.category = category;
    event.level = level;
    event.function = function;
    event.line = line;
    event.message = message;
    return event;
}

/* Helper to create function entry event */
DebugEvent debug_create_enter_event(int category, const char *function, 
                                   const char *file, int line, const char *message) {
    DebugEvent event = debug_create_event(DEBUG_EVENT_ENTER, category, 
                                         DEBUG_LEVEL_ENTER, function, line, message);
    event.data.func_event.function_name = function;
    event.data.func_event.location = file;
    event.data.func_event.duration_ms = 0;
    return event;
}

/* Helper to create function exit event */
DebugEvent debug_create_exit_event(int category, const char *function, 
                                  const char *file, int line, const char *message, 
                                  long duration_ms) {
    DebugEvent event = debug_create_event(DEBUG_EVENT_EXIT, category, 
                                         DEBUG_LEVEL_EXIT, function, line, message);
    event.data.func_event.function_name = function;
    event.data.func_event.location = file;
    event.data.func_event.duration_ms = duration_ms;
    return event;
}

/* Helper to create variable watch event */
DebugEvent debug_create_variable_event(int category, const char *function, int line,
                                      const char *var_name, const char *old_value, 
                                      const char *new_value) {
    DebugEvent event = debug_create_event(DEBUG_EVENT_VARIABLE, category, 
                                         DEBUG_LEVEL_INFO, function, line, "Variable changed");
    event.data.var_event.var_name = var_name;
    event.data.var_event.old_value = old_value;
    event.data.var_event.new_value = new_value;
    return event;
}

/* Helper to create breakpoint event */
DebugEvent debug_create_breakpoint_event(int category, const char *function, int line,
                                        int breakpoint_id, BreakType break_type,
                                        const char *condition, int hit_count) {
    DebugEvent event = debug_create_event(DEBUG_EVENT_BREAKPOINT, category, 
                                         DEBUG_LEVEL_INFO, function, line, "Breakpoint hit");
    event.data.break_event.breakpoint_id = breakpoint_id;
    event.data.break_event.break_type = break_type;
    event.data.break_event.condition = condition;
    event.data.break_event.hit_count = hit_count;
    return event;
}

/* Helper to create performance event */
DebugEvent debug_create_performance_event(int category, const char *function, int line,
                                         const char *counter_name, long duration_ms,
                                         int call_count, long total_ms) {
    DebugEvent event = debug_create_event(DEBUG_EVENT_PERFORMANCE, category,
                                         DEBUG_LEVEL_INFO, function, line, "Performance measurement");
    event.data.perf_event.counter_name = counter_name;
    event.data.perf_event.duration_ms = duration_ms;
    event.data.perf_event.call_count = call_count;
    event.data.perf_event.total_ms = total_ms;
    return event;
}

/* Helper to create call stack event */
DebugEvent debug_create_stack_event(int category, const char *function, int line,
                                   int stack_depth, CallFrame *current_frame) {
    DebugEvent event = debug_create_event(DEBUG_EVENT_STACK, category,
                                         DEBUG_LEVEL_INFO, function, line, "Call stack changed");
    event.data.stack_event.stack_depth = stack_depth;
    event.data.stack_event.current_frame = current_frame;
    return event;
}

/* Clean up event subscriptions */
void debug_events_cleanup(void) {
    EventSubscription *sub = event_subscriptions;
    while (sub != NULL) {
        EventSubscription *next = sub->next;
        debug_free_subscription(sub);
        sub = next;
    }
    event_subscriptions = NULL;
}

/* Free a subscription structure */
static void debug_free_subscription(EventSubscription *sub) {
    if (sub != NULL) {
        free(sub);
    }
}

#endif /* ES_DEBUG */