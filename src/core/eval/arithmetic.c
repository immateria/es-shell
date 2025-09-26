/* arithmetic.c -- arithmetic expression parsing and evaluation */

#include "es.h" 
#include "eval/arithmetic.h"

/* Check if a string represents a compound arithmetic expression */
extern int is_arithmetic_expression(char *name) {
	size_t name_len = strlen(name);
	
	/* Only process compound expressions: digit + operator + [sign +] digit */
	for (size_t i = 1; i < name_len - 1; i++) {
		if ((name[i] == '+' || name[i] == '-' || name[i] == '*' || name[i] == '/') &&
		    isdigit(name[i-1])) {
			/* Check if followed by digit or sign+digit */
			if (isdigit(name[i+1])) {
				/* Standard case: digit + operator + digit */
				return 1;
			} else if ((name[i+1] == '+' || name[i+1] == '-') && 
			          i+2 < name_len && isdigit(name[i+2])) {
				/* Signed operand case: digit + operator + sign + digit */
				return 1;
			}
		}
	}
	
	/* Check for literal numbers like "5", "-3.14", "0" */
	char *endptr;
	double parsed_value = strtod(name, &endptr);
	(void)parsed_value; /* Suppress unused variable warning */
	
	/* If strtod consumed the entire string, it's a valid number */
	return (*endptr == '\0' && endptr != name) ? 1 : 0;
}

/* Parse and evaluate an arithmetic expression */
extern struct List *parse_arithmetic_expression(char *name, struct List *next_args) {
	size_t name_len = strlen(name);
	
	/* First check for compound expressions: digit + operator + [sign +] digit */
	for (size_t i = 1; i < name_len - 1; i++) {
		if ((name[i] == '+' || name[i] == '-' || name[i] == '*' || name[i] == '/') &&
		    isdigit(name[i-1])) {
			/* Check if followed by digit or sign+digit */
			Boolean valid_compound = 0;
			if (isdigit(name[i+1])) {
				/* Standard case: digit + operator + digit */
				valid_compound = 1;
			} else if ((name[i+1] == '+' || name[i+1] == '-') && 
			          i+2 < name_len && isdigit(name[i+2])) {
				/* Signed operand case: digit + operator + sign + digit */
				valid_compound = 1;
			}
			
			if (valid_compound) {
				char *op_pos = name + i;
				char detected_op = name[i];
				char *prim_name;
				
				if (detected_op == '+') {
					prim_name = "%addition";
				} else if (detected_op == '-') {
					prim_name = "%subtraction";
				} else if (detected_op == '*') {
					prim_name = "%multiplication";
				} else if (detected_op == '/') {
					prim_name = "%division";
				} else {
					break; /* Should not happen */
				}
				
				/* Extract operands */
				size_t left_len = op_pos - name;
				char *left_operand = ealloc(left_len + 1);
				memcpy(left_operand, name, left_len);
				left_operand[left_len] = '\0';
				
				char *right_operand = str("%s", op_pos + 1);
				
				/* Create arithmetic call: primitive left_operand right_operand */
				return mklist(mkstr(prim_name), 
				             mklist(mkstr(left_operand), 
				                   mklist(mkstr(right_operand), next_args)));
			}
		}
	}
	
	/* Check for literal numbers like "5", "-3.14", "0" */
	char *endptr;
	double parsed_value = strtod(name, &endptr);
	(void)parsed_value; /* Suppress unused variable warning */
	
	/* If strtod consumed the entire string, it's a valid number */
	if (*endptr == '\0' && endptr != name) {
		/* Return the number as a literal string value */
		return mklist(mkstr(name), next_args);
	}
	
	/* Not an arithmetic expression */
	return NULL;
}