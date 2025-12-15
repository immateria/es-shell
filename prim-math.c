/* prim-math.c -- mathematical and bitwise primitives */

#include "es.h"
#include "prim.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static Boolean try_parse_long(const char *string, long *result) {
    char *end;
    long parsed;

    errno = 0;
    parsed = strtol(string, &end, 0);

    if (*string == '\0' || end == string || *end != '\0' || errno == ERANGE)
        return FALSE;

    *result = parsed;
    return TRUE;
}

static double require_double(const char *string, const char *primitive, const char *context) {
    char *end;
    double parsed;

    errno = 0;
    parsed = strtod(string, &end);

    if (*string == '\0' || end == string || *end != '\0' || errno == ERANGE)
        fail(primitive, "%s", context);

    return parsed;
}

static long require_integer(const char *string, const char *primitive, const char *context) {
    long parsed;

    if (!try_parse_long(string, &parsed))
        fail(primitive, "%s must be an integer: %s", context, string);

    return parsed;
}

/*
 * Arithmetic Operations
 */

PRIM(addition)
{   double result = 0.0;

    for (List *lp = list; lp != NULL; lp = lp->next) {
        double operand = require_double(getstr(lp->term), "$&addition", "arguments must be numbers");

        result += operand;
    }
    return mklist(mkstr(str("%g", result)), NULL);
}

PRIM(subtraction)
{   double result;

    if (list == NULL || list->next == NULL)
        fail("$&subtraction", "usage: $&subtraction number number [...]");

    result = require_double(getstr(list->term), "$&subtraction", "arguments must be numbers");

    for (list = list->next; list != NULL; list = list->next) {
        double operand = require_double(getstr(list->term), "$&subtraction", "arguments must be numbers");

        result -= operand;
    }
    return mklist(mkstr(str("%g", result)), NULL);
}

PRIM(multiplication)
{   double result = 1.0;

    if (list == NULL)
        fail("$&multiplication", "usage: $&multiplication number [...]");

    for (List *lp = list; lp != NULL; lp = lp->next) {
        double operand = require_double(getstr(lp->term), "$&multiplication", "arguments must be numbers");

        result *= operand;
    }
    return mklist(mkstr(str("%g", result)), NULL);
}

PRIM(division)
{   double result;

    if (list == NULL || list->next == NULL)
        fail("$&division", "usage: $&division dividend divisor [...]");

    result = require_double(getstr(list->term), "$&division", "arguments must be numbers");

    for (list = list->next; list != NULL; list = list->next) {
        double divisor = require_double(getstr(list->term), "$&division", "arguments must be numbers");

        if (divisor == 0.0)
            fail("$&division", "division by zero");

        result /= divisor;
    }
    return mklist(mkstr(str("%g", result)), NULL);
}

PRIM(modulo)
{   double dividend;
    double divisor;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&modulo", "usage: $&modulo dividend divisor");

    dividend = require_double(getstr(list->term), "$&modulo", "arguments must be numbers");

    list = list->next;
    divisor = require_double(getstr(list->term), "$&modulo", "arguments must be numbers");

    if (divisor == 0.0)
        fail("$&modulo", "division by zero");

    return mklist(mkstr(str("%g", fmod(dividend, divisor))), NULL);
}

PRIM(pow)
{   double base_value;
    double exponent_value;
    double result;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&pow", "usage: $&pow base exponent");

    base_value = require_double(getstr(list->term), "$&pow", "base must be a number");

    exponent_value = require_double(getstr(list->next->term), "$&pow", "exponent must be a number");

    if (base_value == 0.0 && exponent_value < 0.0)
        fail("$&pow", "zero cannot be raised to a negative power");

    result = pow(base_value, exponent_value);

    return mklist(mkstr(str("%g", result)), NULL);
}

PRIM(abs)
{   double input_value;
 
    if (list == NULL || list->next != NULL)
        fail("$&abs", "usage: $&abs number");
    
    input_value = require_double(getstr(list->term), "$&abs", "argument must be a number");
    
    return mklist(mkstr(str("%g", fabs(input_value))), NULL);
}

PRIM(min)
{   double minimum_value;

    if (list == NULL)
        fail("$&min", "usage: $&min number [number ...]");

    minimum_value = require_double(getstr(list->term), "$&min", "arguments must be numbers");

    for (list = list->next; list != NULL; list = list->next) {
        double current_value = require_double(getstr(list->term), "$&min", "arguments must be numbers");

        if (current_value < minimum_value)
            minimum_value = current_value;
    }
    return mklist(mkstr(str("%g", minimum_value)), NULL);
}

PRIM(max)
{   double maximum_value;

    if (list == NULL)
        fail("$&max", "usage: $&max number [number ...]");

    maximum_value = require_double(getstr(list->term), "$&max", "arguments must be numbers");

    for (list = list->next; list != NULL; list = list->next) {
        double current_value = require_double(getstr(list->term), "$&max", "arguments must be numbers");

        if (current_value > maximum_value)
            maximum_value = current_value;
    }
    return mklist(mkstr(str("%g", maximum_value)), NULL);
}

PRIM(count)
{   return mklist(mkstr(str("%d", length(list))), NULL);
}

/*
 * Bitwise Operations
 */

PRIM(bitwiseshiftleft)
{   long  input_value;
    long  shift_amount;
    const int max_shift_bits = sizeof(long) * 8 - 1;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&bitwiseshiftleft", "usage: $&bitwiseshiftleft value shift_amount");

    input_value = require_integer(getstr(list->term), "$&bitwiseshiftleft", "value argument");

    shift_amount = require_integer(getstr(list->next->term), "$&bitwiseshiftleft", "shift_amount argument");

    if (shift_amount < 0)
        fail("$&bitwiseshiftleft", "shift_amount cannot be negative");

    if (shift_amount > max_shift_bits)
        fail("$&bitwiseshiftleft", "shift_amount too large (maximum %d bits)", max_shift_bits);

    return mklist(mkstr(str("%ld", input_value << shift_amount)), NULL);
}

PRIM(bitwiseshiftright)
{   long  input_value;
    long  shift_amount;
    const int max_shift_bits = sizeof(long) * 8 - 1;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&bitwiseshiftright", "usage: $&bitwiseshiftright value shift_amount");

    input_value = require_integer(getstr(list->term), "$&bitwiseshiftright", "value argument");

    shift_amount = require_integer(getstr(list->next->term), "$&bitwiseshiftright", "shift_amount argument");

    if (shift_amount < 0)
        fail("$&bitwiseshiftright", "shift_amount cannot be negative");

    if (shift_amount > max_shift_bits)
        fail("$&bitwiseshiftright", "shift_amount too large (maximum %d bits)", max_shift_bits);

    return mklist(mkstr(str("%ld", input_value >> shift_amount)), NULL);
}

PRIM(bitwiseand)
{   long  result;
 
    if (list == NULL)
        fail("$&bitwiseand", "usage: $&bitwiseand number [number ...]");
    
    result = require_integer(getstr(list->term), "$&bitwiseand", "each argument");
 
    for (list = list->next; list != NULL; list = list->next)
    {   long operand = require_integer(getstr(list->term), "$&bitwiseand", "each argument");

        result &= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(bitwiseor)
{   long  result = 0;
 
    for (List *lp = list; lp != NULL; lp = lp->next)
    {   long operand = require_integer(getstr(lp->term), "$&bitwiseor", "each argument");

        result |= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(bitwisexor)
{   long  result = 0;
 
    for (List *lp = list; lp != NULL; lp = lp->next)
    {   long operand = require_integer(getstr(lp->term), "$&bitwisexor", "each argument");

        result ^= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(bitwisenot)
{   long  input_value;
 
    if (list == NULL || list->next != NULL)
        fail("$&bitwisenot", "usage: $&bitwisenot number");
    
    input_value = require_integer(getstr(list->term), "$&bitwisenot", "argument");
    
    return mklist(mkstr(str("%ld", ~input_value)), NULL);
}

/*
 * Comparison Operations
 */

PRIM(greater)
{   double first, second;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&greater", "usage: $&greater number1 number2");

    first = require_double(getstr(list->term), "$&greater", "arguments must be numbers");

    second = require_double(getstr(list->next->term), "$&greater", "arguments must be numbers");

    return first > second ? ltrue : lfalse;
}

PRIM(less)
{   double first, second;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&less", "usage: $&less number1 number2");

    first = require_double(getstr(list->term), "$&less", "arguments must be numbers");

    second = require_double(getstr(list->next->term), "$&less", "arguments must be numbers");

    return first < second ? ltrue : lfalse;
}

PRIM(greaterequal)
{   double first, second;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&greaterequal", "usage: $&greaterequal number1 number2");

    first = require_double(getstr(list->term), "$&greaterequal", "arguments must be numbers");

    second = require_double(getstr(list->next->term), "$&greaterequal", "arguments must be numbers");

    return first >= second ? ltrue : lfalse;
}

PRIM(lessequal)
{   double first, second;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&lessequal", "usage: $&lessequal number1 number2");

    first = require_double(getstr(list->term), "$&lessequal", "arguments must be numbers");

    second = require_double(getstr(list->next->term), "$&lessequal", "arguments must be numbers");

    return first <= second ? ltrue : lfalse;
}

PRIM(equal)
{   double first, second;
    const double epsilon = 1e-15;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&equal", "usage: $&equal number1 number2");

    first = require_double(getstr(list->term), "$&equal", "arguments must be numbers");

    second = require_double(getstr(list->next->term), "$&equal", "arguments must be numbers");

    return fabs(first - second) < epsilon ? ltrue : lfalse;
}

PRIM(notequal)
{   double first, second;
    const double epsilon = 1e-15;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&notequal", "usage: $&notequal number1 number2");

    first = require_double(getstr(list->term), "$&notequal", "arguments must be numbers");

    second = require_double(getstr(list->next->term), "$&notequal", "arguments must be numbers");

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
        double d = require_double(getstr(list->term), "$&toint", "argument must be a number");
        result = (long)d;
    }

    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(tofloat)
{   double result;

    if (list == NULL || list->next != NULL)
        fail("$&tofloat", "usage: $&tofloat number");

    result = require_double(getstr(list->term), "$&tofloat", "argument must be a number");

    return mklist(mkstr(str("%g", result)), NULL);
}

PRIM(isint)
{   long value;

    if (list == NULL || list->next != NULL)
        fail("$&isint", "usage: $&isint value");

    return try_parse_long(getstr(list->term), &value) ? ltrue : lfalse;
}

PRIM(isfloat)
{   char *endptr;
    double dummy;
    const char *str_val = getstr(list->term);

    if (list == NULL || list->next != NULL)
        fail("$&isfloat", "usage: $&isfloat value");

    errno = 0;
    dummy = strtod(str_val, &endptr);
    (void)dummy; /* suppress unused variable warning */

    if (errno == ERANGE)
        return lfalse;

    /* It's a float if it parses as a number AND contains a decimal point */
    return (endptr != NULL && *endptr != '\0') ? lfalse :
           (strchr(str_val, '.') != NULL) ? ltrue : lfalse;
}

/*
 * Integer-only Arithmetic Operations
 */

PRIM(intaddition)
{   long result = 0;

    for (List *lp = list; lp != NULL; lp = lp->next) {
        long operand = require_integer(getstr(lp->term), "$&intaddition", "each argument");

        result += operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(intsubtraction)
{   long result;

    if (list == NULL || list->next == NULL)
        fail("$&intsubtraction", "usage: $&intsubtraction number number [...]");

    result = require_integer(getstr(list->term), "$&intsubtraction", "each argument");

    for (list = list->next; list != NULL; list = list->next) {
        long operand = require_integer(getstr(list->term), "$&intsubtraction", "each argument");

        result -= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(intmultiplication)
{   long result = 1;

    if (list == NULL)
        fail("$&intmultiplication", "usage: $&intmultiplication number [...]");

    for (List *lp = list; lp != NULL; lp = lp->next) {
        long operand = require_integer(getstr(lp->term), "$&intmultiplication", "each argument");

        result *= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(intdivision)
{   long result;

    if (list == NULL || list->next == NULL)
        fail("$&intdivision", "usage: $&intdivision dividend divisor [...]");

    result = require_integer(getstr(list->term), "$&intdivision", "each argument");

    for (list = list->next; list != NULL; list = list->next) {
        long divisor = require_integer(getstr(list->term), "$&intdivision", "each argument");

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
    X(bitwiseand);
    X(bitwiseor);
    X(bitwisexor);
    X(bitwisenot);
    
    /* Comparison operations */
    X(greater);
    X(less);
    X(greaterequal);
    X(lessequal);
    X(equal);
    X(notequal);
    
    return primdict;
}
