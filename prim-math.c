/* prim-math.c -- mathematical and bitwise primitives */

#include "es.h"
#include "prim.h"

#include <limits.h>
#include <stdio.h>

/*
 * Arithmetic Operations
 */

PRIM(addition)
{   long result = 0;

    for (List *lp = list; lp != NULL; lp = lp->next)
    {   char *endptr;
        long  operand = strtol(getstr(lp->term), &endptr, 0);

        if (endptr != NULL && *endptr != '\0')
            fail("$&addition", "arguments must be integers");

        result += operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(subtraction)
{   char *endptr;
    long  result;

    if (list == NULL || list->next == NULL)
        fail("$&subtraction", "usage: $&subtraction number number [...]");

    result = strtol(getstr(list->term), &endptr, 0);

    if (endptr != NULL && *endptr != '\0')
        fail("$&subtraction", "arguments must be integers");

    for (list = list->next; list != NULL; list = list->next)
    {   long operand = strtol(getstr(list->term), &endptr, 0);

        if (endptr != NULL && *endptr != '\0')
            fail("$&subtraction", "arguments must be integers");

        result -= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(multiplication)
{   char *endptr;
    long  result = 1;

    if (list == NULL)
        fail("$&multiplication", "usage: $&multiplication number [...]");

    for (List *lp = list; lp != NULL; lp = lp->next)
    {   long operand = strtol(getstr(lp->term), &endptr, 0);

        if (endptr != NULL && *endptr != '\0')
            fail("$&multiplication", "arguments must be integers");

        result *= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(division)
{   char *endptr;
    long  result;

    if (list == NULL || list->next == NULL)
        fail("$&division", "usage: $&division dividend divisor [...]");

    result = strtol(getstr(list->term), &endptr, 0);

    if (endptr != NULL && *endptr != '\0')
        fail("$&division", "arguments must be integers");

    for (list = list->next; list != NULL; list = list->next)
    {   long divisor = strtol(getstr(list->term), &endptr, 0);

        if (endptr != NULL && *endptr != '\0')
            fail("$&division", "arguments must be integers");

        if (divisor == 0)
            fail("$&division", "division by zero");

        result /= divisor;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(modulo)
{   char *endptr;
    long  dividend;
    long  divisor;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&modulo", "usage: $&modulo dividend divisor");

    dividend = strtol(getstr(list->term), &endptr, 0);

    if (endptr != NULL && *endptr != '\0')
        fail("$&modulo", "arguments must be integers");

    list = list->next;
    divisor = strtol(getstr(list->term), &endptr, 0);

    if (endptr != NULL && *endptr != '\0')
        fail("$&modulo", "arguments must be integers");

    if (divisor == 0)
        fail("$&modulo", "division by zero");

    return mklist(mkstr(str("%ld", dividend % divisor)), NULL);
}

PRIM(pow)
{   char *endptr;
    long  base_value;
    long  exponent_value;
    long  integer_result = 1;

    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&pow", "usage: $&pow base exponent");

    base_value = strtol(getstr(list->term), &endptr, 0);

    if (endptr != NULL && *endptr != '\0')
        fail("$&pow", "base must be an integer");

    exponent_value = strtol(getstr(list->next->term), &endptr, 0);

    if (endptr != NULL && *endptr != '\0')
        fail("$&pow", "exponent must be an integer");

    if (exponent_value < 0) {
        unsigned long magnitude;
        double fractional_result = 1.0;
        double base_as_double;
        char buffer[128];
        int written;

        if (base_value == 0)
            fail("$&pow", "zero cannot be raised to a negative power");
        if (exponent_value == LONG_MIN)
            fail("$&pow", "exponent magnitude too large");

        magnitude = (unsigned long) -exponent_value;
        base_as_double = (double) base_value;

        for (unsigned long i = 0; i < magnitude; i++)
            fractional_result /= base_as_double;

        if (fractional_result == 0.0)
            return mklist(mkstr(str("0")), NULL);

        written = snprintf(buffer, sizeof buffer, "%.15g", fractional_result);

        if (written < 0 || written >= (int) sizeof buffer)
            fail("$&pow", "unable to format fractional result");

        return mklist(mkstr(str("%s", buffer)), NULL);
    }

    for (long i = 0; i < exponent_value; i++)
        integer_result *= base_value;

    return mklist(mkstr(str("%ld", integer_result)), NULL);
}

PRIM(abs)
{   char *endptr;
    long  input_value;
 
    if (list == NULL || list->next != NULL)
        fail("$&abs", "usage: $&abs number");
    
    input_value = strtol(getstr(list->term), &endptr, 0);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&abs", "argument must be an integer");
    
    return mklist(mkstr(str("%ld", input_value < 0 ? -input_value : input_value)), NULL);
}

PRIM(min)
{   char *endptr;
    long  minimum_value;
 
    if (list == NULL)
        fail("$&min", "usage: $&min number [number ...]");
    
    minimum_value = strtol(getstr(list->term), &endptr, 0);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&min", "arguments must be integers");
 
    for (list = list->next; list != NULL; list = list->next)
    {   long current_value = strtol(getstr(list->term), &endptr, 0);
     
        if (endptr != NULL && *endptr != '\0')
            fail("$&min", "arguments must be integers");
        
        if (current_value < minimum_value)
            minimum_value = current_value;
    }
    return mklist(mkstr(str("%ld", minimum_value)), NULL);
}

PRIM(max)
{   char *endptr;
    long  maximum_value;
 
    if (list == NULL)
        fail("$&max", "usage: $&max number [number ...]");
    
    maximum_value = strtol(getstr(list->term), &endptr, 0);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&max", "arguments must be integers");
 
    for (list = list->next; list != NULL; list = list->next)
    {   long current_value = strtol(getstr(list->term), &endptr, 0);
     
        if (endptr != NULL && *endptr != '\0')
            fail("$&max", "arguments must be integers");
        
        if (current_value > maximum_value)
            maximum_value = current_value;
    }
    return mklist(mkstr(str("%ld", maximum_value)), NULL);
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
    
    return primdict;
}
