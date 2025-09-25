/* token-utils.c -- character classification utilities for ES tokenizer */

#include "es.h"
#include "token-utils.h"
#include <string.h>

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
extern void init_char_tables(void) {
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

/*
 * Helper function to check if character is an octal digit.
 * Used in escape sequence processing.
 */
extern int is_octal_digit(int c) {
	return '0' <= c && c < '8';
}