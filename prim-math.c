/* prim-math.c -- mathematical and bitwise primitives */

#include "es.h"
#include "prim.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>

/*
 * Arithmetic Operations
 */

PRIM(addition)
{   double result = 0.0;

    for (List *lp = list; lp != NULL; lp = lp->next)
    {   char *endptr;
        double operand = strtod(getstr(lp->term), &endptr);

        if (endptr != NULL && *endptr != '\0')
            fail("$&addition", "arguments must be numbers");

        result += operand;
    }
    return mklist(mkstr(str("%g", result)), NULL);
}

PRIM(subtraction)
{   char *endptr;
    double result;

    if (list == NULL || list->next == NULL)
        fail("$&subtraction", "usage: $&subtraction number number [...]");

    result = strtod(getstr(list->term), &endptr);

    if (endptr != NULL && *endptr != '\0')
        fail("$&subtraction", "arguments must be numbers");

    for (list = list->next; list != NULL; list = list->next)
    {   double operand = strtod(getstr(list->term), &endptr);

        if (endptr != NULL && *endptr != '\0')
            fail("$&subtraction", "arguments must be numbers");

        result -= operand;
    }
    return mklist(mkstr(str("%g", result)), NULL);
}

PRIM(multiplication)
{   char *endptr;
    double result = 1.0;

    if (list == NULL)
        fail("$&multiplication", "usage: $&multiplication number [...]");

    for (List *lp = list; lp != NULL; lp = lp->next)
    {   double operand = strtod(getstr(lp->term), &endptr);

        if (endptr != NULL && *endptr != '\0')
            fail("$&multiplication", "arguments must be numbers");

        result *= operand;
    }
    return mklist(mkstr(str("%g", result)), NULL);
}

PRIM(division)
{   char *endptr;
    double result;

    if (list == NULL || list->next == NULL)
        fail("$&division", "usage: $&division dividend divisor [...]");

    result = strtod(getstr(list->term), &endptr);

    if (endptr != NULL && *endptr != '\0')
        fail("$&division", "arguments must be numbers");

    for (list = list->next; list != NULL; list = list->next)
    {   double divisor = strtod(getstr(list->term), &endptr);

        if (endptr != NULL && *endptr != '\0')
            fail("$&division", "arguments must be numbers");

        if (divisor == 0.0)
            fail("$&division", "division by zero");

        result /= divisor;
    }
    return mklist(mkstr(str("%g", result)), NULL);
}

PRIM(modulo)
{   char *endptr;
    double dividend;
    double divisor;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&modulo", "usage: $&modulo dividend divisor");

    dividend = strtod(getstr(list->term), &endptr);

    if (endptr != NULL && *endptr != '\0')
        fail("$&modulo", "arguments must be numbers");

    list = list->next;
    divisor = strtod(getstr(list->term), &endptr);

    if (endptr != NULL && *endptr != '\0')
        fail("$&modulo", "arguments must be numbers");

    if (divisor == 0.0)
        fail("$&modulo", "division by zero");

    return mklist(mkstr(str("%g", fmod(dividend, divisor))), NULL);
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

    return mklist(mkstr(str("%g", result)), NULL);
}

PRIM(abs)
{   char *endptr;
    double input_value;
 
    if (list == NULL || list->next != NULL)
        fail("$&abs", "usage: $&abs number");
    
    input_value = strtod(getstr(list->term), &endptr);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&abs", "argument must be a number");
    
    return mklist(mkstr(str("%g", fabs(input_value))), NULL);
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
    return mklist(mkstr(str("%g", minimum_value)), NULL);
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
    return mklist(mkstr(str("%g", maximum_value)), NULL);
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
        fail("$&greater", "arguments must be numbers");

    second = strtod(getstr(list->next->term), &endptr);
    if (endptr != NULL && *endptr != '\0')
        fail("$&greater", "arguments must be numbers");

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
