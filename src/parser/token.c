/* token.c -- lexical analyzer for es ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "input.h"
#include "syntax.h"
#include "token.h"
#include "token-utils.h"
#include "token-redir.h"
#include "token-string.h"
#include <string.h>

#define	isodigit(c)	is_octal_digit(c)

#define	BUFSIZE	((size_t) 2048)
#define	BUFMAX	(8 * BUFSIZE)

typedef enum { NW, RW, KW } State;	/* "nonword", "realword", "keyword" */

static State w = NW;
static Boolean qword = FALSE;
static Boolean newline = FALSE;
static Boolean goterror = FALSE;
size_t bufsize = 0;
char *tokenbuf = NULL;

#define	InsertFreeCaret()	STMT(if (w != NW) { w = NW; UNGETC(c); return '^'; })




/* print_prompt2 -- called before all continuation lines */
extern void print_prompt2(void) {
	input->lineno++;
#if HAVE_READLINE
	prompt = prompt2;
#else
	if ((input->runflags & run_interactive) && prompt2 != NULL)
		eprint("%s", prompt2);
#endif
}

/* scanerror -- called for lexical errors */
extern void scanerror(int c, char *s) {
	while (c != '\n' && c != EOF)
		c = GETC();
	goterror = TRUE;
	yyerror(s);
}

/*
 * getfds
 *	Scan in a pair of integers for redirections like >[2=1]. CLOSED represents
 *	a closed file descriptor (i.e., >[2=]).
 *
 *	This function makes use of unsigned compares to make range tests in
 *	one compare operation.
 */

#define	CLOSED	-1
#define	DEFAULT	-2

static Boolean getfds(int fd[2], int c, int default0, int default1) {
	int n;
	fd[0] = default0;
	fd[1] = default1;

	if (c != '[') {
		UNGETC(c);
		return TRUE;
	}
	if ((unsigned int) (n = GETC() - '0') > 9) {
		scanerror(n + '0', "expected digit after '['");
		return FALSE;
	}

	while ((unsigned int) (c = GETC() - '0') <= 9)
		n = n * 10 + c;
	fd[0] = n;

	switch (c += '0') {
	case '=':
		if ((unsigned int) (n = GETC() - '0') > 9) {
			if (n != ']' - '0') {
				scanerror(n + '0', "expected digit or ']' after '='");
				return FALSE;
			}
			fd[1] = CLOSED;
		} else {
			while ((unsigned int) (c = GETC() - '0') <= 9)
				n = n * 10 + c;
			if (c != ']' - '0') {
				scanerror(c + '0', "expected ']' after digit");
				return FALSE;
			}
			fd[1] = n;
		}
		break;
	case ']':
		break;
	default:
		scanerror(c, "expected '=' or ']' after digit");
		return FALSE;
	}
	return TRUE;
}

/* Helper function to check if character is non-word in current context */
static int is_meta_char(int c, Boolean in_dollar_context) {
	return in_dollar_context ? is_dollar_nonword_char(c) : is_nonword_char(c);
}

extern int yylex(void) {
	static Boolean dollar = FALSE;
	int c;
	size_t i;			/* The purpose of all these local assignments is to	*/
	char *buf = tokenbuf;		/* allow optimizing compilers like gcc to load these	*/
	YYSTYPE *y = &yylval;		/* values into registers. On a sparc this is a		*/
	Boolean in_dollar_context;	/* win, in code size *and* execution time		*/

	/* Initialize character classification tables on first call */
	init_char_tables();

	if (goterror) {
		goterror = FALSE;
		return NL;
	}

	/* rc variable-names may contain only alnum, '*' and '_', so use dollar context check if we are scanning one. */
	in_dollar_context = dollar;
	dollar = FALSE;
	if (newline) {
		--input->lineno; /* slight space optimization; print_prompt2() always increments lineno */
		print_prompt2();
		newline = FALSE;
	}
top:	while ((c = GETC()) == ' ' || c == '	') {
		w = NW;
		qword = FALSE;
	}
	if (c == EOF)
		return ENDFILE;
        /* Special handling for '-' - check if it's part of a redirection operator */
        if (c == '-') {
                int lookahead = GETC();
                if (lookahead == '>') {
                        /* Could be ->, ->!, ->>, ->-<, or ->>< */
                        UNGETC(lookahead);
                        /* Fall through to switch statement to handle these cases */
                } else {
                        /* This is a regular minus in a word, treat as word character */
                        UNGETC(lookahead);
                        InsertFreeCaret();
                        w = RW;
                        qword = FALSE;
                        i = 0;
			Boolean numeric = isdigit(c);
			do {
				buf[i++] = c;
				if (i >= bufsize)
					buf = tokenbuf = erealloc(buf, bufsize *= 2);
				c = GETC();
			} while (c != EOF && (!is_meta_char(c, in_dollar_context) || (!numeric && (c == '-' || c == '*' || c == '+'))));
			UNGETC(c);
			buf[i] = '\0';
			w = KW;
                        if (buf[1] == '\0') {
                                int k = *buf;
                                if (k == '@' || k == '~')
                                        return k;
                        } else if (*buf == 'f') {
                                if (streq(buf + 1, "n"))        return FN;
                                if (streq(buf + 1, "or"))       return FOR;
                        } else if (*buf == 'l') {
                                if (streq(buf + 1, "ocal"))     return LOCAL;
                                if (streq(buf + 1, "et"))       return LET;
                        } else if (streq(buf, "~~")) {
                                return EXTRACT;
                        } else if (streq(buf, "%closure")) {
                                return CLOSURE;
                        } else if (streq(buf, "match")) {
                                return MATCH;
                        }
                        w = RW;
                        y->str = pdup(buf);
                        return WORD;
                }
        } else if (!is_meta_char(c, in_dollar_context) || (!in_dollar_context && (c == '*' || c == '+'))) { /* it's a word or keyword. */
                InsertFreeCaret();
                w = RW;
                qword = FALSE;
                i = 0;
		Boolean numeric = isdigit(c);
		do {
			buf[i++] = c;
			if (i >= bufsize)
				buf = tokenbuf = erealloc(buf, bufsize *= 2);
			c = GETC();
		} while (c != EOF && (!is_meta_char(c, in_dollar_context) || (!numeric && (c == '-' || c == '*' || c == '+'))));
		UNGETC(c);
		buf[i] = '\0';
		w = KW;
                if (buf[1] == '\0') {
                        int k = *buf;
                        if (k == '@' || k == '~')
                                return k;
                } else if (*buf == 'f') {
                        if (streq(buf + 1, "n"))        return FN;
                        if (streq(buf + 1, "or"))       return FOR;
                } else if (*buf == 'l') {
                        if (streq(buf + 1, "ocal"))     return LOCAL;
                        if (streq(buf + 1, "et"))       return LET;
                } else if (streq(buf, "~~")) {
                        return EXTRACT;
                } else if (streq(buf, "%closure")) {
                        return CLOSURE;
                } else if (streq(buf, "match")) {
                        return MATCH;
                }
                w = RW;
                y->str = pdup(buf);
                return WORD;
        }
	
	/* Check for assignment operators with lookahead */
	int enter_switch = 0;
	if (c == '+' || c == '*' || c == '/' || c == '.') {
		int next = GETC();
		if (next == '=') {
			/* This is an assignment operator */
			UNGETC(next);
			enter_switch = 1;
		} else {
			/* Not an assignment operator - put back and process normally */
			UNGETC(next);
			enter_switch = 0;
		}
	} else if (c == '`' || c == '!' || c == '$' || c == '\'' || c == '=') {
		enter_switch = 1;
	}
	
	if (enter_switch) {
		InsertFreeCaret();
		if (c == '!' || c == '=')
			w = KW;
	}
	switch (c) {
	case '!':
		c = GETC();
		if (c == '=') {
			/* != not equal */
			w = NW;
			return NE;
		}
		UNGETC(c);
		return '!';
	case '=':
		c = GETC();
		if (c == '=') {
			/* == equal */
			w = NW;
			return EQ;
		}
		UNGETC(c);
		return '=';
	case '`':
		c = GETC();
		if (c == '`') {
			c = GETC();
			if (c == '^')
				return BBFLAT;
			UNGETC(c);
			return BACKBACK;
		} else if (c == '^')
			return BFLAT;
		UNGETC(c);
		return '`';
	case '$':
		dollar = TRUE;
		switch (c = GETC()) {
		case '#':	return COUNT;
		case '^':	return FLAT;
                case '&':       return PRIM;
		case '{':	return EXPR_CALL;  /* ${...} expression evaluation */
		default:	UNGETC(c); return '$';
		}
        case '\'':
                w = RW;
                qword = TRUE;
                return parse_quoted_string();
	case '\\': {
		Boolean continue_line = FALSE;
		InsertFreeCaret();
		w = RW;
		int result = handle_backslash_line_continuation(&continue_line);
		if (continue_line) {
			goto top; /* Pretend it was just another space. */
		}
		return result;
	}
	case '#':
		while ((c = GETC()) != '\n') /* skip comment until newline */
			if (c == EOF)
				return ENDFILE;
		FALLTHROUGH;
	case '\n':
		input->lineno++;
		newline = TRUE;
		w = NW;
		return NL;
	case '(':
		if (w == RW)	/* not keywords, so let & friends work */
			c = SUB;
		FALLTHROUGH;
        case ';':
        case '^':
        case ')':
        case '{': case '}':
                w = NW;
                return c;

	case '&':
		w = NW;
		c = GETC();
		if (c == '&')
			return ANDAND;
		UNGETC(c);
		return '&';

	case '|': {
		int p[2];
		w = NW;
		c = GETC();
		if (c == '|')
			return OROR;
		if (!getfds(p, c, 1, 0))
			return ERROR;
		if (p[1] == CLOSED) {
			scanerror(c, "expected digit after '='");	/* can't close a pipe */
			return ERROR;
		}
		y->tree = mk(nPipe, p[0], p[1]);
		return PIPE;
	}

	{
		/* Variables used for old redirection syntax, now unused */
		/* char *cmd; */
		/* int fd[2]; */
        case '<':
                w = NW;
                return parse_less_than_operators();
        case '>':
                w = NW;
                return parse_greater_than_operators();
        
        case '-': {
                Boolean process_as_word = FALSE;
                int token = parse_minus_operators(&process_as_word);
                if (process_as_word) {
                        c = '-';
                        goto top;
                } else {
                        w = NW;
                        return token;
                }
        }
	/* This code block was originally used for old redirection syntax
	 * but is no longer needed with the new arrow-based syntax.
	 * Keeping it here commented out for reference.
	 */
	#if 0
	redirection:
		w = NW;
		if (!getfds(fd, c, fd[0], DEFAULT))
			return ERROR;
		if (fd[1] != DEFAULT) {
			y->tree = (fd[1] == CLOSED)
					? mkclose(fd[0])
					: mkdup(fd[0], fd[1]);
			return DUP;
		}
		y->tree = mkredircmd(cmd, fd[0]);
		return REDIR;
	#endif
	}

	default:
		assert(c != '\0');
		w = NW;
		return c; /* don't know what it is, let yacc barf on it */
	}
}

extern void inityy(void) {
	newline = FALSE;
	w = NW;
	if (bufsize > BUFMAX) {		/* return memory to the system if the buffer got too large */
		efree(tokenbuf);
		tokenbuf = NULL;
	}
	if (tokenbuf == NULL) {
		bufsize = BUFSIZE;
		tokenbuf = ealloc(bufsize);
	}
}
