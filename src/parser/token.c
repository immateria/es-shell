/* token.c -- lexical analyzer for es ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "input.h"
#include "syntax.h"
#include "token.h"
#include <string.h>

#define	isodigit(c)	('0' <= (c) && (c) < '8')

#define	BUFSIZE	((size_t) 2048)
#define	BUFMAX	(8 * BUFSIZE)

typedef enum { NW, RW, KW } State;	/* "nonword", "realword", "keyword" */

static State w = NW;
static Boolean qword = FALSE;
static Boolean newline = FALSE;
static Boolean goterror = FALSE;
static size_t bufsize = 0;
static char *tokenbuf = NULL;

#define	InsertFreeCaret()	STMT(if (w != NW) { w = NW; UNGETC(c); return '^'; })


/* 
 * Character classification for tokenizing.
 * 
 * ES shell syntax special characters that break words in normal context.
 * These are the core ES shell metacharacters and delimiters.
 */
static const unsigned char ES_SPECIAL_CHARS[] = {
	'\0', '\t', '\n', ' ', '!', '#', '$', '&', '\'', '(', ')', '*', '+', '-', ';', '<', '=', '>', '\\', '^', '`', '{', '|', '}'
};
#define ES_SPECIAL_CHARS_COUNT (sizeof(ES_SPECIAL_CHARS))

/*
 * Characters allowed in variable names within $... expressions.
 * Much more restrictive than normal context - only alphanumeric, underscore,
 * and a few special variable characters (%, *, .).
 */
static const char DOLLAR_WORD_CHARS[] = 
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_*%.";

/* Lookup tables for fast character classification during tokenization */
static char nonword_chars[256];
static char dollar_nonword_chars[256];

/*
 * Initialize character classification lookup tables.
 * Called once at startup to build the tables from semantic definitions.
 */
static void init_char_tables(void) {
	static int initialized = 0;
	if (initialized) return;
	
	/* Initialize all characters as word characters (0) */
	memset(nonword_chars, 0, sizeof(nonword_chars));
	memset(dollar_nonword_chars, 1, sizeof(dollar_nonword_chars)); /* default non-word for $ context */
	
	/* Mark ES special characters as non-word in normal context */
	for (int i = 0; i < (int)ES_SPECIAL_CHARS_COUNT; i++) {
		nonword_chars[ES_SPECIAL_CHARS[i]] = 1;
	}
	
	/* Mark allowed characters as word characters in dollar context */
	for (const char *p = DOLLAR_WORD_CHARS; *p; p++) {
		dollar_nonword_chars[(unsigned char)*p] = 0;
	}
	
	initialized = 1;
}

/*
 * Check if character is a non-word character in normal context.
 * Used by external modules like conv.c.
 */
extern int is_nonword_char(int c) {
	init_char_tables();  /* Ensure tables are initialized */
	return (c >= 0 && c < 256) ? nonword_chars[c] : 1;
}

/*
 * Check if character is a non-word character in dollar context.
 * Used by external modules like heredoc.c.
 */
extern int is_dollar_nonword_char(int c) {
	init_char_tables();  /* Ensure tables are initialized */
	return (c >= 0 && c < 256) ? dollar_nonword_chars[c] : 1;
}


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
static void scanerror(int c, char *s) {
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

extern int yylex(void) {
	static Boolean dollar = FALSE;
	int c;
	size_t i;			/* The purpose of all these local assignments is to	*/
	const char *meta;		/* allow optimizing compilers like gcc to load these	*/
	char *buf = tokenbuf;		/* values into registers. On a sparc this is a		*/
	YYSTYPE *y = &yylval;		/* win, in code size *and* execution time		*/

	/* Initialize character classification tables on first call */
	init_char_tables();

	if (goterror) {
		goterror = FALSE;
		return NL;
	}

	/* rc variable-names may contain only alnum, '*' and '_', so use dollar_nonword_chars if we are scanning one. */
	meta = (dollar ? dollar_nonword_chars : nonword_chars);
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
			} while (c != EOF && (!meta[(unsigned char) c] || (!numeric && (c == '-' || c == '*' || c == '+'))));
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
        } else if (!meta[(unsigned char) c] || c == '*' || c == '+') { /* it's a word or keyword. */
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
		} while (c != EOF && (!meta[(unsigned char) c] || (!numeric && (c == '-' || c == '*' || c == '+'))));
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
	if (c == '`' || c == '!' || c == '$' || c == '\'' || c == '=') {
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
                i = 0;
		while ((c = GETC()) != '\'' || (c = GETC()) == '\'') {
			buf[i++] = c;
			if (c == '\n')
				print_prompt2();
			if (c == EOF) {
				w = NW;
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
	case '\\':
		if ((c = GETC()) == '\n') {
			print_prompt2();
			UNGETC(' ');
			goto top; /* Pretend it was just another space. */
		}
		if (c == EOF) {
			UNGETC(EOF);
			goto badescape;
		}
		UNGETC(c);
		c = '\\';
		InsertFreeCaret();
		w = RW;
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
			} while (isodigit(c));
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
                /* < is now ONLY a comparison operator */
                /* Use <- for input redirection */
                c = GETC();
                if (c == '-') {
                        c = GETC();
                        if (c == '-') {
                                c = GETC();
                                if (c == '<') {
                                        /* <--< heredoc syntax */
                                        w = NW;
                                        return HEREDOC_NEW;
                                }
                                /* Not heredoc, treat as comparison */
                                UNGETC(c);
                                UNGETC('-');
                                UNGETC('-');
                                w = NW;
                                return LT;  /* Less than operator */
                } else if (c == '!') {
                                /* <-! forced input redirection */
                                w = NW;
                                return FLARROW;
                        } else if (c == '>') {
                                c = GETC();
                                if (c == '>') {
                                        /* <->> open-append redirection */
                                        w = NW;
                                        return OA_ARROW;
                                } else {
                                        /* <-> read-write redirection */
                                        UNGETC(c);
                                        w = NW;
                                        return RW_ARROW;
                                }
                        } else {
                                /* <- input redirection */
                                UNGETC(c);
                                w = NW;
                                return LARROW;
                        }
                } else if (c == '=') {
                        /* <= less than or equal */
                        w = NW;
                        return LE;
                } else {
                        /* < less than comparison */
                        UNGETC(c);
                        w = NW;
                        return LT;
                }
        case '>':
                /* > is now ONLY a comparison operator */
                /* Use -> for output redirection */
                c = GETC();
                if (c == '=') {
                        /* >= greater than or equal */
                        w = NW;
                        return GE;
                } else {
                        /* > greater than comparison */
                        UNGETC(c);
                        w = NW;
                        return GT;
                }
        
        case '-':
                /* Check for -> and ->! output redirection */
                c = GETC();
                if (c == '>') {
                        c = GETC();
                        if (c == '!') {
                                /* ->! forced output redirection */
                                w = NW;
                                return FRARROW;
                        } else if (c == '>') {
                                c = GETC();
                                if (c == '<') {
                                        /* ->>< append with create */
                                        w = NW;
                                        return APPEND_CREATE;
                                } else {
                                        /* ->> append redirection */
                                        UNGETC(c);
                                        w = NW;
                                        return APPEND_ARROW;
                                }
                        } else if (c == '-') {
                                c = GETC();
                                if (c == '<') {
                                        /* ->-< open-create redirection */
                                        w = NW;
                                        return OC_ARROW;
                                } else {
                                        /* Not a valid operator */
                                        UNGETC(c);
                                        UNGETC('-');
                                        /* -> output redirection */
                                        w = NW;
                                        return RARROW;
                                }
                        } else {
                                /* -> output redirection */
                                UNGETC(c);
                                w = NW;
                                return RARROW;
                        }
                } else {
                        /* Just a regular minus sign - let it be processed as a word */
                        UNGETC(c);
                        UNGETC('-');
                        c = '-';
                        goto top;
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
