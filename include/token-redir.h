/* token-redir.h -- redirection operator parsing for ES tokenizer */

#ifndef TOKEN_REDIR_H
#define TOKEN_REDIR_H

/*
 * Parse '<' and related redirection operators.
 * Handles: <, <=, <-, <-!, <->, <->>, <--<
 * Returns appropriate token type (LT, LE, LARROW, FLARROW, etc.)
 */
extern int parse_less_than_operators(void);

/*
 * Parse '>' and related operators.  
 * Handles: >, >=
 * Returns GT or GE (comparison operators only)
 */
extern int parse_greater_than_operators(void);

/*
 * Parse '-' and related redirection operators.
 * Handles: ->, ->!, ->>, ->>< , ->-<
 * Sets *process_as_word to TRUE if this should be handled as a regular minus.
 * Returns appropriate token when process_as_word is FALSE.
 */
extern int parse_minus_operators(Boolean *process_as_word);

#endif /* TOKEN_REDIR_H */