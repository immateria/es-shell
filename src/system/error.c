/* error.c -- Enhanced error handling system for ES Shell */

#include "es.h"
#include "print.h"
#include <limits.h>

typedef enum {
    ERROR_SYNTAX,
    ERROR_TYPE,
    ERROR_ARGUMENT,
    ERROR_RUNTIME,
    ERROR_IO,
    ERROR_SYSTEM
} ErrorCategory;

typedef struct {
    ErrorCategory category;
    const char *title;
    const char *message;
    const char *suggestion;
    const char *example;
} EnhancedError;

static const char *error_category_names[] = {
    "Syntax Error",
    "Type Error", 
    "Argument Error",
    "Runtime Error",
    "I/O Error",
    "System Error"
};

/* Format and display an enhanced error message */
static void print_enhanced_error(const char *context, EnhancedError *error) {
    print("\n🚫 %s: %s\n", error_category_names[error->category], error->title);
    print("   Context: %s\n", context);
    print("   Problem: %s\n", error->message);
    
    if (error->suggestion) {
        print("   💡 Suggestion: %s\n", error->suggestion);
    }
    
    if (error->example) {
        print("   📝 Example: %s\n", error->example);
    }
    
    print("\n");
}

/* Enhanced fail function with better error messages */
extern Noreturn enhanced_fail(const char *context, ErrorCategory category, 
                              const char *title, const char *message,
                              const char *suggestion, const char *example) {
    EnhancedError error = {
        .category = category,
        .title = title,
        .message = message,
        .suggestion = suggestion,
        .example = example
    };
    
    print_enhanced_error(context, &error);
    
    // Still throw the traditional exception for compatibility
    gcdisable();
    Ref(List *, e, mklist(mkstr("error"),
                         mklist(mkstr((char *) context),
                               mklist(mkstr((char *) message), NULL))));
    while (gcisblocked())
        gcenable();
    throw(e);
    RefEnd(e);
}

/* Convenient wrappers for common error types */

extern Noreturn fail_syntax(const char *context, const char *message, const char *suggestion) {
    enhanced_fail(context, ERROR_SYNTAX, "Invalid Syntax", message, suggestion, NULL);
}

extern Noreturn fail_type(const char *context, const char *expected, const char *got, const char *example) {
    char *message = str("Expected %s, got %s", expected, got);
    enhanced_fail(context, ERROR_TYPE, "Type Mismatch", message, 
                  "Check the argument types", example);
}

extern Noreturn fail_args(const char *context, const char *usage, const char *got) {
    char *message = str("Invalid arguments: %s", got ? got : "wrong number or type");
    enhanced_fail(context, ERROR_ARGUMENT, "Invalid Arguments", message, 
                  NULL, usage);
}

extern Noreturn fail_runtime(const char *context, const char *message, const char *suggestion) {
    enhanced_fail(context, ERROR_RUNTIME, "Runtime Error", message, suggestion, NULL);
}

extern Noreturn fail_math(const char *context, const char *operation, const char *reason) {
    char *message = str("%s failed: %s", operation, reason);
    enhanced_fail(context, ERROR_RUNTIME, "Math Error", message,
                  "Check your input values", NULL);
}

extern Noreturn fail_io(const char *context, const char *filename, const char *operation) {
    char *message = str("Cannot %s file '%s'", operation, filename);
    enhanced_fail(context, ERROR_IO, "File Operation Failed", message,
                  "Check file permissions and path", NULL);
}

/* Smart argument validation with helpful errors */
extern void validate_arg_count(const char *context, List *args, int min, int max, const char *usage) {
    int count = length(args);
    
    if (count < min) {
        char *got = str("got %d argument%s", count, count == 1 ? "" : "s");
        fail_args(context, usage, got);
    }
    
    if (max >= 0 && count > max) {
        char *got = str("got %d argument%s (too many)", count, count == 1 ? "" : "s");
        fail_args(context, usage, got);
    }
}

extern double validate_number(const char *context, const char *str_value, const char *arg_name) {
    char *endptr;
    double value = strtod(str_value, &endptr);
    
    if (endptr && *endptr != '\0') {
        char *message = str("'%s' is not a valid number", str_value);
        char *suggestion = str("Use a numeric value for %s", arg_name);
        enhanced_fail(context, ERROR_TYPE, "Invalid Number", message, suggestion, 
                      "echo ${5 plus 3.14}");
    }
    
    return value;
}

extern int validate_integer(const char *context, const char *str_value, const char *arg_name) {
    char *endptr;
    long value = strtol(str_value, &endptr, 10);
    
    if (endptr && *endptr != '\0') {
        char *message = str("'%s' is not a valid integer", str_value);
        char *suggestion = str("Use a whole number for %s", arg_name);
        enhanced_fail(context, ERROR_TYPE, "Invalid Integer", message, suggestion, 
                      "count = 5");
    }
    
    if (value > INT_MAX || value < INT_MIN) {
        enhanced_fail(context, ERROR_RUNTIME, "Number Too Large", 
                      "Integer value is out of range",
                      "Use a smaller number", NULL);
    }
    
    return (int)value;
}

/* Validate mathematical operations */
extern void validate_not_zero(const char *context, double value, const char *operation) {
    if (value == 0.0) {
        fail_math(context, operation, "division by zero");
    }
}

extern void validate_positive(const char *context, double value, const char *operation) {
    if (value <= 0.0) {
        char *reason = str("negative or zero input: %g", value);
        fail_math(context, operation, reason);
    }
}

/* Help users with common mistakes */
extern Noreturn suggest_alternative(const char *context, const char *attempted, 
                                    const char *suggestion, const char *reason) {
    char *message = str("'%s' is not available", attempted);
    enhanced_fail(context, ERROR_SYNTAX, "Unknown Command", message, 
                  str("Try '%s' instead (%s)", suggestion, reason), 
                  str("%s example usage here", suggestion));
}