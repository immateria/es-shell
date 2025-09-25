/* token-string.c -- string literal and escape sequence processing for ES tokenizer */

#include "es.h"
#include "input.h"
#include "token-string.h"
#include "token-utils.h"
#include "token.h"
#include <ctype.h>

/* External declarations needed from token.c */
extern YYSTYPE yylval;

/*
 * Parse a quoted string literal (single quotes).
 * Handles quote escaping ('' becomes ') and multi-line strings.
 * Returns QWORD token with the processed string.
 */
extern int parse_quoted_string(void) {
	int c;
	size_t i = 0;
	char *buf = tokenbuf;
	YYSTYPE *y = &yylval;

	while ((c = GETC()) != '\'' || (c = GETC()) == '\'') {
		buf[i++] = c;
		if (c == '\n')
			print_prompt2();
		if (c == EOF) {
			scanerror(c, "eof in quoted string");
			return ERROR;
		}
		if (i >= bufsize)
			buf = tokenbuf = erealloc(buf, bufsize *= 2);
	}
	UNGETC(c);
	buf[i] = '\0';
	y->str = pdup(buf);
	return QWORD;
}

/*
 * Process a backslash escape sequence.
 * Handles standard escapes (\n, \t, etc), octal (\123), and hex (\xFF).
 * Returns QWORD token with the escaped character.
 */
extern int parse_escape_sequence(void) {
	int c;
	char *buf = tokenbuf;
	YYSTYPE *y = &yylval;

	c = GETC();
	switch (c) {
	case 'a':	*buf = '\a';	break;
	case 'b':	*buf = '\b';	break;
	case 'e':	*buf = '\033';	break;
	case 'f':	*buf = '\f';	break;
	case 'n':	*buf = '\n';	break;
	case 'r':	*buf = '\r';	break;
	case 't':	*buf = '\t';	break;
	case 'x': case 'X': {
		int n = 0;
		for (;;) {
			c = GETC();
			if (!isxdigit(c))
				break;
			n = (n << 4)
			  | (c - (isdigit(c) ? '0' : ((islower(c) ? 'a' : 'A') - 0xA)));
		}
		if (n == 0)
			goto badescape;
		UNGETC(c);
		*buf = n;
		break;
	}
	case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': {
		int n = 0;
		do {
			n = (n << 3) | (c - '0');
			c = GETC();
		} while (is_octal_digit(c));
		if (n == 0)
			goto badescape;
		UNGETC(c);
		*buf = n;
		break;
	}
	default:
		if (isalnum(c)) {
		badescape:
			scanerror(c, "bad backslash escape");
			return ERROR;
		}
		*buf = c;
		break;
	}
	buf[1] = 0;
	y->str = pdup(buf);
	return QWORD;
}

/*
 * Handle backslash at beginning of line (line continuation).
 * If followed by newline, treats it as whitespace.
 * Otherwise, processes as escape sequence.
 * Returns appropriate token or signals line continuation.
 */
extern int handle_backslash_line_continuation(Boolean *continue_line) {
	int c = GETC();
	if (c == '\n') {
		print_prompt2();
		UNGETC(' ');
		*continue_line = TRUE;
		return 0; /* Return value ignored when continue_line is TRUE */
	}
	if (c == EOF) {
		UNGETC(EOF);
		scanerror(c, "bad backslash escape");
		return ERROR;
	}
	UNGETC(c);
	*continue_line = FALSE;
	return parse_escape_sequence();
}