/* token-utils.h -- character classification utilities for ES tokenizer */

#ifndef TOKEN_UTILS_H
#define TOKEN_UTILS_H

/*
 * Initialize character classification lookup tables.
 * Must be called before using other functions in this module.
 */
extern void init_char_tables(void);

/*
 * Check if character is a non-word character in normal context.
 * Returns 1 if the character breaks words, 0 if it's part of words.
 */
extern int is_nonword_char(int c);

/*
 * Check if character is a non-word character in dollar variable context.
 * Returns 1 if the character breaks variable names, 0 if it's allowed.
 */
extern int is_dollar_nonword_char(int c);

/*
 * Helper function to check if character is an octal digit (0-7).
 * Used in escape sequence processing.
 */
extern int is_octal_digit(int c);

#endif /* TOKEN_UTILS_H */