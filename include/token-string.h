/* token-string.h -- string literal and escape sequence processing for ES tokenizer */

#ifndef TOKEN_STRING_H
#define TOKEN_STRING_H

/* Shared variables from token.c needed by string processing */
extern char *tokenbuf;
extern size_t bufsize;

/* Functions from token.c needed by string processing */
extern void print_prompt2(void);
extern void scanerror(int c, char *s);

/*
 * Parse a quoted string literal (single quotes).
 * Handles quote escaping ('' becomes ') and multi-line strings.
 * Returns QWORD token with the processed string.
 */
extern int parse_quoted_string(void);

/*
 * Process a backslash escape sequence.
 * Handles standard escapes (\n, \t, etc), octal (\123), and hex (\xFF).
 * Returns QWORD token with the escaped character.
 */
extern int parse_escape_sequence(void);

/*
 * Handle backslash at beginning of line (line continuation).
 * If followed by newline, treats it as whitespace.
 * Otherwise, processes as escape sequence.
 * Sets *continue_line to TRUE if this should be treated as line continuation.
 */
extern int handle_backslash_line_continuation(Boolean *continue_line);

#endif /* TOKEN_STRING_H */