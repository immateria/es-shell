/* prim-math.c -- mathematical and bitwise primitives */

#include "es.h"
#include "prim.h"
#include "error.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

/*
 * Helper functions for optimal integer/float arithmetic
 */

/* Check if a string represents an integer within long range */
static Boolean is_safe_integer(const char *str, long *result) {
    char *endptr;
    long value = strtol(str, &endptr, 10);
    
    if (endptr && *endptr == '\0') {
        *result = value;
        return TRUE;
    }
    return FALSE;
}

/* Smart number formatting - display whole numbers as integers, others as floats */
static char* format_smart_number(double value) {
    /* Check if the value is mathematically a whole number */
    if (value == floor(value) && !isinf(value) && !isnan(value)) {
        /* Display as integer if it's a whole number within long range */
        if (value >= LONG_MIN && value <= LONG_MAX) {
            return str("%ld", (long)value);
        }
    }
    /* Display as float for fractional numbers or very large integers */
    return str("%g", value);
}

/* Fast integer arithmetic with overflow detection */
static Boolean safe_add_integers(long a, long b, long *result) {
    if ((b > 0 && a > LONG_MAX - b) || (b < 0 && a < LONG_MIN - b)) {
        return FALSE; /* Overflow */
    }
    *result = a + b;
    return TRUE;
}

static Boolean safe_multiply_integers(long a, long b, long *result) {
    if (a == 0 || b == 0) {
        *result = 0;
        return TRUE;
    }
    if ((a > 0 && b > 0 && a > LONG_MAX / b) ||
        (a < 0 && b < 0 && a < LONG_MAX / b) ||
        (a > 0 && b < 0 && b < LONG_MIN / a) ||
        (a < 0 && b > 0 && a < LONG_MIN / b)) {
        return FALSE; /* Overflow */
    }
    *result = a * b;
    return TRUE;
}

/*
 * Arithmetic Operations
 */

PRIM(addition)
{   validate_arg_count("addition", list, 1, -1, "addition number [number ...]");

    /* Try fast integer path first */
    Boolean all_integers = TRUE;
    long int_result = 0;
    
    for (List *lp = list; lp != NULL; lp = lp->next) {
        long operand;
        if (!is_safe_integer(getstr(lp->term), &operand)) {
            all_integers = FALSE;
            break;
        }
        if (!safe_add_integers(int_result, operand, &int_result)) {
            all_integers = FALSE; /* Overflow, fall back to float */
            break;
        }
    }
    
    if (all_integers) {
        return mklist(mkstr(str("%ld", int_result)), NULL);
    }
    
    /* Fall back to floating-point arithmetic */
    double result = 0.0;
    for (List *lp = list; lp != NULL; lp = lp->next) {
        double operand = validate_number("addition", getstr(lp->term), "operand");
        result += operand;
    }
    return mklist(mkstr(format_smart_number(result)), NULL);
}

PRIM(subtraction)
{   double result;

    if (list == NULL || list->next == NULL)
        fail("$&subtraction", "usage: $&subtraction number number [...]");

    result = validate_number("subtraction", getstr(list->term), "minuend");

    for (list = list->next; list != NULL; list = list->next)
    {   double operand = validate_number("subtraction", getstr(list->term), "operand");
        result -= operand;
    }
    return mklist(mkstr(format_smart_number(result)), NULL);
}

PRIM(multiplication)
{   if (list == NULL)
        fail("$&multiplication", "usage: $&multiplication number [...]");

    /* Try fast integer path first */
    Boolean all_integers = TRUE;
    long int_result = 1;
    
    for (List *lp = list; lp != NULL; lp = lp->next) {
        long operand;
        if (!is_safe_integer(getstr(lp->term), &operand)) {
            all_integers = FALSE;
            break;
        }
        if (!safe_multiply_integers(int_result, operand, &int_result)) {
            all_integers = FALSE; /* Overflow, fall back to float */
            break;
        }
    }
    
    if (all_integers) {
        return mklist(mkstr(str("%ld", int_result)), NULL);
    }
    
    /* Fall back to floating-point arithmetic */
    double result = 1.0;
    for (List *lp = list; lp != NULL; lp = lp->next) {
        double operand = validate_number("multiplication", getstr(lp->term), "operand");
        result *= operand;
    }
    return mklist(mkstr(format_smart_number(result)), NULL);
}

PRIM(division)
{   double result;

    validate_arg_count("division", list, 2, -1, "division dividend divisor [divisor ...]");

    result = validate_number("division", getstr(list->term), "dividend");

    for (list = list->next; list != NULL; list = list->next)
    {   double divisor = validate_number("division", getstr(list->term), "divisor");
        validate_not_zero("division", divisor, "division");
        result /= divisor;
    }
    return mklist(mkstr(format_smart_number(result)), NULL);
}

PRIM(modulo)
{   double dividend;
    double divisor;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&modulo", "usage: $&modulo dividend divisor");

    dividend = validate_number("modulo", getstr(list->term), "dividend");
    divisor = validate_number("modulo", getstr(list->next->term), "divisor");

    if (divisor == 0.0)
        fail("$&modulo", "division by zero");

    return mklist(mkstr(format_smart_number(fmod(dividend, divisor))), NULL);
}

PRIM(pow)
{   char *endptr;
    double base_value;
    double exponent_value;
    double result;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&pow", "usage: $&pow base exponent");

    base_value = strtod(getstr(list->term), &endptr);

    if (endptr != NULL && *endptr != '\0')
        fail("$&pow", "base must be a number");

    exponent_value = strtod(getstr(list->next->term), &endptr);

    if (endptr != NULL && *endptr != '\0')
        fail("$&pow", "exponent must be a number");

    if (base_value == 0.0 && exponent_value < 0.0)
        fail("$&pow", "zero cannot be raised to a negative power");

    result = pow(base_value, exponent_value);

    return mklist(mkstr(str("%f", result)), NULL);
}

PRIM(abs)
{   char *endptr;
    double input_value;
 
    if (list == NULL || list->next != NULL)
        fail("$&abs", "usage: $&abs number");
    
    input_value = strtod(getstr(list->term), &endptr);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&abs", "argument must be a number");
    
    return mklist(mkstr(str("%f", fabs(input_value))), NULL);
}

PRIM(min)
{   char *endptr;
    double minimum_value;
 
    if (list == NULL)
        fail("$&min", "usage: $&min number [number ...]");
    
    minimum_value = strtod(getstr(list->term), &endptr);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&min", "arguments must be numbers");
 
    for (list = list->next; list != NULL; list = list->next)
    {   double current_value = strtod(getstr(list->term), &endptr);
     
        if (endptr != NULL && *endptr != '\0')
            fail("$&min", "arguments must be numbers");
        
        if (current_value < minimum_value)
            minimum_value = current_value;
    }
    return mklist(mkstr(str("%f", minimum_value)), NULL);
}

PRIM(max)
{   char *endptr;
    double maximum_value;
 
    if (list == NULL)
        fail("$&max", "usage: $&max number [number ...]");
    
    maximum_value = strtod(getstr(list->term), &endptr);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&max", "arguments must be numbers");
 
    for (list = list->next; list != NULL; list = list->next)
    {   double current_value = strtod(getstr(list->term), &endptr);
     
        if (endptr != NULL && *endptr != '\0')
            fail("$&max", "arguments must be numbers");
        
        if (current_value > maximum_value)
            maximum_value = current_value;
    }
    return mklist(mkstr(str("%f", maximum_value)), NULL);
}

PRIM(count)
{   return mklist(mkstr(str("%d", length(list))), NULL);
}

/*
 * Bitwise Operations
 */

PRIM(bitwiseshiftleft)
{   char *endptr_for_value;
    char *endptr_for_shift;
    long  input_value;
    long  shift_amount;
    const int max_shift_bits = sizeof(long) * 8 - 1;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&bitwiseshiftleft", "usage: $&bitwiseshiftleft value shift_amount");

    input_value = strtol(getstr(list->term), &endptr_for_value, 0);

    if (endptr_for_value != NULL && *endptr_for_value != '\0')
        fail("$&bitwiseshiftleft", "value argument must be an integer");

    shift_amount = strtol(getstr(list->next->term), &endptr_for_shift, 0);

    if (endptr_for_shift != NULL && *endptr_for_shift != '\0')
        fail("$&bitwiseshiftleft", "shift_amount argument must be an integer");

    if (shift_amount < 0)
        fail("$&bitwiseshiftleft", "shift_amount cannot be negative");

    if (shift_amount > max_shift_bits)
        fail("$&bitwiseshiftleft", "shift_amount too large (maximum %d bits)", max_shift_bits);

    return mklist(mkstr(str("%ld", input_value << shift_amount)), NULL);
}

PRIM(bitwiseshiftright)
{   char *endptr_for_value;
    char *endptr_for_shift;
    long  input_value;
    long  shift_amount;
    const int max_shift_bits = sizeof(long) * 8 - 1;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&bitwiseshiftright", "usage: $&bitwiseshiftright value shift_amount");

    input_value = strtol(getstr(list->term), &endptr_for_value, 0);

    if (endptr_for_value != NULL && *endptr_for_value != '\0')
        fail("$&bitwiseshiftright", "value argument must be an integer");

    shift_amount = strtol(getstr(list->next->term), &endptr_for_shift, 0);

    if (endptr_for_shift != NULL && *endptr_for_shift != '\0')
        fail("$&bitwiseshiftright", "shift_amount argument must be an integer");

    if (shift_amount < 0)
        fail("$&bitwiseshiftright", "shift_amount cannot be negative");

    if (shift_amount > max_shift_bits)
        fail("$&bitwiseshiftright", "shift_amount too large (maximum %d bits)", max_shift_bits);

    return mklist(mkstr(str("%ld", input_value >> shift_amount)), NULL);
}

PRIM(and)
{   char *endptr;
    long  result;
 
    if (list == NULL)
        fail("$&and", "usage: $&and number [number ...]");
    
    result = strtol(getstr(list->term), &endptr, 0);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&and", "arguments must be integers");
 
    for (list = list->next; list != NULL; list = list->next)
    {   long operand = strtol(getstr(list->term), &endptr, 0);
     
        if (endptr != NULL && *endptr != '\0')
            fail("$&and", "arguments must be integers");
        
        result &= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(or)
{   char *endptr;
    long  result = 0;
 
    for (List *lp = list; lp != NULL; lp = lp->next)
    {   long operand = strtol(getstr(lp->term), &endptr, 0);
     
        if (endptr != NULL && *endptr != '\0')
            fail("$&or", "arguments must be integers");
        
        result |= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(xor)
{   char *endptr;
    long  result = 0;
 
    for (List *lp = list; lp != NULL; lp = lp->next)
    {   long operand = strtol(getstr(lp->term), &endptr, 0);
     
        if (endptr != NULL && *endptr != '\0')
            fail("$&xor", "arguments must be integers");
        
        result ^= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(not)
{   char *endptr;
    long  input_value;
 
    if (list == NULL || list->next != NULL)
        fail("$&not", "usage: $&not number");
    
    input_value = strtol(getstr(list->term), &endptr, 0);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&not", "argument must be an integer");
    
    return mklist(mkstr(str("%ld", ~input_value)), NULL);
}

/*
 * Comparison Operations
 */

PRIM(greater)
{   char *endptr;
    double first, second;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&greater", "usage: $&greater number1 number2");

    first = strtod(getstr(list->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&greater", "first argument must be a number");

    second = strtod(getstr(list->next->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&greater", "second argument must be a number");

    return first > second ? ltrue : lfalse;
}

PRIM(less)
{   char *endptr;
    double first, second;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&less", "usage: $&less number1 number2");

    first = strtod(getstr(list->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&less", "arguments must be numbers");

    second = strtod(getstr(list->next->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&less", "arguments must be numbers");

    return first < second ? ltrue : lfalse;
}

PRIM(greaterequal)
{   char *endptr;
    double first, second;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&greaterequal", "usage: $&greaterequal number1 number2");

    first = strtod(getstr(list->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&greaterequal", "arguments must be numbers");

    second = strtod(getstr(list->next->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&greaterequal", "arguments must be numbers");

    return first >= second ? ltrue : lfalse;
}

PRIM(lessequal)
{   char *endptr;
    double first, second;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&lessequal", "usage: $&lessequal number1 number2");

    first = strtod(getstr(list->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&lessequal", "arguments must be numbers");

    second = strtod(getstr(list->next->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&lessequal", "arguments must be numbers");

    return first <= second ? ltrue : lfalse;
}

PRIM(equal)
{   char *endptr;
    double first, second;
    const double epsilon = 1e-15;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&equal", "usage: $&equal number1 number2");

    first = strtod(getstr(list->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&equal", "arguments must be numbers");

    second = strtod(getstr(list->next->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&equal", "arguments must be numbers");

    return fabs(first - second) < epsilon ? ltrue : lfalse;
}

PRIM(notequal)
{   char *endptr;
    double first, second;
    const double epsilon = 1e-15;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&notequal", "usage: $&notequal number1 number2");

    first = strtod(getstr(list->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&notequal", "arguments must be numbers");

    second = strtod(getstr(list->next->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&notequal", "arguments must be numbers");

    return fabs(first - second) >= epsilon ? ltrue : lfalse;
}

/*
 * Type Conversion Operations
 */

PRIM(toint)
{   char *endptr;
    long result;

    if (list == NULL || list->next != NULL)
        fail("$&toint", "usage: $&toint number");

    result = strtol(getstr(list->term), &endptr, 0);
    if (endptr != NULL && *endptr != '\0') {
        /* Try parsing as float first, then convert to int */
        double d = strtod(getstr(list->term), &endptr);
        if (endptr != NULL && *endptr != '\0')
            fail("$&toint", "argument must be a number");
        result = (long)d;
    }

    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(tofloat)
{   char *endptr;
    double result;

    if (list == NULL || list->next != NULL)
        fail("$&tofloat", "usage: $&tofloat number");

    result = strtod(getstr(list->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&tofloat", "argument must be a number");

    return mklist(mkstr(str("%f", result)), NULL);
}

PRIM(isint)
{   char *endptr;
    long dummy;

    if (list == NULL || list->next != NULL)
        fail("$&isint", "usage: $&isint value");

    dummy = strtol(getstr(list->term), &endptr, 0);
    (void)dummy; /* suppress unused variable warning */
    
    return (endptr != NULL && *endptr != '\0') ? lfalse : ltrue;
}

PRIM(isfloat)
{   char *endptr;
    double dummy;
    const char *str_val = getstr(list->term);

    if (list == NULL || list->next != NULL)
        fail("$&isfloat", "usage: $&isfloat value");

    dummy = strtod(str_val, &endptr);
    (void)dummy; /* suppress unused variable warning */
    
    /* It's a float if it parses as a number AND contains a decimal point */
    return (endptr != NULL && *endptr != '\0') ? lfalse : 
           (strchr(str_val, '.') != NULL) ? ltrue : lfalse;
}

/*
 * Integer-only Arithmetic Operations
 */

PRIM(intaddition)
{   char *endptr;
    long result = 0;

    for (List *lp = list; lp != NULL; lp = lp->next) {
        long operand = strtol(getstr(lp->term), &endptr, 0);

        if (endptr != NULL && *endptr != '\0')
            fail("$&intaddition", "arguments must be integers");

        result += operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(intsubtraction)
{   char *endptr;
    long result;

    if (list == NULL || list->next == NULL)
        fail("$&intsubtraction", "usage: $&intsubtraction number number [...]");

    result = strtol(getstr(list->term), &endptr, 0);
    if (endptr != NULL && *endptr != '\0')
        fail("$&intsubtraction", "arguments must be integers");

    for (list = list->next; list != NULL; list = list->next) {
        long operand = strtol(getstr(list->term), &endptr, 0);

        if (endptr != NULL && *endptr != '\0')
            fail("$&intsubtraction", "arguments must be integers");

        result -= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(intmultiplication)
{   char *endptr;
    long result = 1;

    if (list == NULL)
        fail("$&intmultiplication", "usage: $&intmultiplication number [...]");

    for (List *lp = list; lp != NULL; lp = lp->next) {
        long operand = strtol(getstr(lp->term), &endptr, 0);

        if (endptr != NULL && *endptr != '\0')
            fail("$&intmultiplication", "arguments must be integers");

        result *= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(intdivision)
{   char *endptr;
    long result;

    if (list == NULL || list->next == NULL)
        fail("$&intdivision", "usage: $&intdivision dividend divisor [...]");

    result = strtol(getstr(list->term), &endptr, 0);
    if (endptr != NULL && *endptr != '\0')
        fail("$&intdivision", "arguments must be integers");

    for (list = list->next; list != NULL; list = list->next) {
        long divisor = strtol(getstr(list->term), &endptr, 0);

        if (endptr != NULL && *endptr != '\0')
            fail("$&intdivision", "arguments must be integers");

        if (divisor == 0)
            fail("$&intdivision", "division by zero");

        result /= divisor;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

/*
 * Initialization
 */

extern Dict *initprims_math(Dict *primdict)
{   /* Arithmetic operations */
    X(addition);
    X(subtraction);
    X(multiplication);
    X(division);
    X(modulo);
    X(pow);
    X(abs);
    X(min);
    X(max);
    X(count);
    
    /* Integer-only arithmetic operations */
    X(intaddition);
    X(intsubtraction);
    X(intmultiplication);
    X(intdivision);
    
    /* Type conversion operations */
    X(toint);
    X(tofloat);
    X(isint);
    X(isfloat);
    
    /* Bitwise operations */
    X(bitwiseshiftleft);
    X(bitwiseshiftright);
    X(and);
    X(or);
    X(xor);
    X(not);
    
    /* Comparison operations */
    X(greater);
    X(less);
    X(greaterequal);
    X(lessequal);
    X(equal);
    X(notequal);
    
    return primdict;
}
