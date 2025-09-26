/* arithmetic.h -- arithmetic expression parsing interface */

#ifndef EVAL_ARITHMETIC_H
#define EVAL_ARITHMETIC_H

/* Forward declarations - these types are defined in es.h */
struct List;

/* Check if a string represents an arithmetic expression */
extern int is_arithmetic_expression(char *name);

/* Parse and evaluate an arithmetic expression, returning the result as a List */
extern struct List *parse_arithmetic_expression(char *name, struct List *next_args);

#endif /* EVAL_ARITHMETIC_H */