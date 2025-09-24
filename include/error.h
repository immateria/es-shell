/* error.h -- Enhanced error handling system declarations */

#ifndef ERROR_H
#define ERROR_H

/* Forward declarations */
typedef struct List List;

#ifndef Noreturn
#define Noreturn void
#endif

/* Error category enumeration */
typedef enum {
    ERROR_SYNTAX,
    ERROR_TYPE,
    ERROR_ARGUMENT,
    ERROR_RUNTIME,
    ERROR_IO,
    ERROR_SYSTEM
} ErrorCategory;

/* Enhanced fail functions */
extern Noreturn enhanced_fail(const char *context, ErrorCategory category, 
                              const char *title, const char *message,
                              const char *suggestion, const char *example);

/* Convenient error type wrappers */
extern Noreturn fail_syntax(const char *context, const char *message, const char *suggestion);
extern Noreturn fail_type(const char *context, const char *expected, const char *got, const char *example);
extern Noreturn fail_args(const char *context, const char *usage, const char *got);
extern Noreturn fail_runtime(const char *context, const char *message, const char *suggestion);
extern Noreturn fail_math(const char *context, const char *operation, const char *reason);
extern Noreturn fail_io(const char *context, const char *filename, const char *operation);

/* Validation helpers */
extern void validate_arg_count(const char *context, List *args, int min, int max, const char *usage);
extern double validate_number(const char *context, const char *str_value, const char *arg_name);
extern int validate_integer(const char *context, const char *str_value, const char *arg_name);
extern void validate_not_zero(const char *context, double value, const char *operation);
extern void validate_positive(const char *context, double value, const char *operation);

/* Smart suggestions */
extern Noreturn suggest_alternative(const char *context, const char *attempted, 
                                    const char *suggestion, const char *reason);

#endif /* ERROR_H */