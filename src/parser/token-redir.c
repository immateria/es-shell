/* token-redir.c -- redirection operator parsing for ES tokenizer */

#include "es.h"
#include "input.h" 
#include "token-redir.h"
#include "token.h"

/*
 * Parse '<' and related redirection operators.
 * Returns appropriate token type (LT, LE, LARROW, FLARROW, etc.)
 * Handles complex multi-character sequences like <--, <-!, <->>, etc.
 */
extern int parse_less_than_operators(void) {
	int c = GETC();
	if (c == '-') {
		c = GETC();
		if (c == '-') {
			c = GETC();
			if (c == '<') {
				/* <--< heredoc syntax */
				return HEREDOC_NEW;
			}
			/* Not heredoc, treat as comparison */
			UNGETC(c);
			UNGETC('-');
			UNGETC('-');
			return LT;  /* Less than operator */
		} else if (c == '!') {
			/* <-! forced input redirection */
			return FLARROW;
		} else if (c == '>') {
			c = GETC();
			if (c == '>') {
				/* <->> open-append redirection */
				return OA_ARROW;
			} else {
				/* <-> read-write redirection */
				UNGETC(c);
				return RW_ARROW;
			}
		} else {
			/* <- input redirection */
			UNGETC(c);
			return LARROW;
		}
	} else if (c == '=') {
		/* <= less than or equal */
		return LE;
	} else {
		/* < less than comparison */
		UNGETC(c);
		return LT;
	}
}

/*
 * Parse '>' and related operators.
 * Returns GT or GE (comparison operators only, no redirections from >)
 */
extern int parse_greater_than_operators(void) {
	int c = GETC();
	if (c == '=') {
		/* >= greater than or equal */
		return GE;
	} else {
		/* > greater than comparison */
		UNGETC(c);
		return GT;
	}
}

/*
 * Parse '-' and related redirection operators.
 * Returns appropriate token or signals to process as regular word.
 * Sets *process_as_word to TRUE if this should be handled as a regular minus.
 */
extern int parse_minus_operators(Boolean *process_as_word) {
	int c = GETC();
	if (c == '>') {
		c = GETC();
		if (c == '!') {
			/* ->! forced output redirection */
			*process_as_word = FALSE;
			return FRARROW;
		} else if (c == '>') {
			c = GETC();
			if (c == '<') {
				/* ->>< append with create */
				*process_as_word = FALSE;
				return APPEND_CREATE;
			} else {
				/* ->> append redirection */
				UNGETC(c);
				*process_as_word = FALSE;
				return APPEND_ARROW;
			}
		} else if (c == '-') {
			c = GETC();
			if (c == '<') {
				/* ->-< open-create redirection */
				*process_as_word = FALSE;
				return OC_ARROW;
			} else {
				/* Not a valid operator */
				UNGETC(c);
				UNGETC('-');
				/* -> output redirection */
				*process_as_word = FALSE;
				return RARROW;
			}
		} else {
			/* -> output redirection */
			UNGETC(c);
			*process_as_word = FALSE;
			return RARROW;
		}
	} else {
		/* Just a regular minus sign - let it be processed as a word */
		UNGETC(c);
		UNGETC('-');
		*process_as_word = TRUE;
		return 0; /* Return value ignored when process_as_word is TRUE */
	}
}