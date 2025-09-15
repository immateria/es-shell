/* prim-math.c -- mathematical and bitwise primitives */

#include "es.h"
#include "prim.h"

/*
 * Arithmetic Operations
 */

PRIM(add)
{   long result = 0;
 
    for (List *lp = list; lp != NULL; lp = lp->next)
    {   char *endptr;
        long  operand = strtol(getstr(lp->term), &endptr, 0);

        if (endptr != NULL && *endptr != '\0')
            fail("$&add", "arguments must be integers");
     
        result += operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(sub)
{   char *endptr;
    long  result;
 
    if (list == NULL || list->next == NULL)
        fail("$&sub", "usage: $&sub number number [...]");
    
    result = strtol(getstr(list->term), &endptr, 0);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&sub", "arguments must be integers");
 
    for (list = list->next; list != NULL; list = list->next)
    {   long operand = strtol(getstr(list->term), &endptr, 0);
     
        if (endptr != NULL && *endptr != '\0')
            fail("$&sub", "arguments must be integers");
            
        result -= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(mul)
{   char *endptr;
    long  result = 1;
 
    if (list == NULL)
        fail("$&mul", "usage: $&mul number [...]");
 
    for (List *lp = list; lp != NULL; lp = lp->next)
    {   long operand = strtol(getstr(lp->term), &endptr, 0);
     
        if (endptr != NULL && *endptr != '\0')
            fail("$&mul", "arguments must be integers");
     
        result *= operand;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(div)
{   char *endptr;
    long  result;
 
    if (list == NULL || list->next == NULL)
        fail("$&div", "usage: $&div dividend divisor [...]");
    
    result = strtol(getstr(list->term), &endptr, 0);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&div", "arguments must be integers");
 
    for (list = list->next; list != NULL; list = list->next)
    {   long divisor = strtol(getstr(list->term), &endptr, 0);
     
        if (endptr != NULL && *endptr != '\0')
            fail("$&div", "arguments must be integers");
        
        if (divisor == 0)
            fail("$&div", "division by zero");
            
        result /= divisor;
    }
    return mklist(mkstr(str("%ld", result)), NULL);
}

PRIM(mod)
{   char *endptr;
    long  dividend;
    long  divisor;
 
    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&mod", "usage: $&mod dividend divisor");
    
    dividend = strtol(getstr(list->term), &endptr, 0);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&mod", "arguments must be integers");
    
    list = list->next;
    divisor = strtol(getstr(list->term), &endptr, 0);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&mod", "arguments must be integers");
    
    if (divisor == 0)
        fail("$&mod", "division by zero");
        
    return mklist(mkstr(str("%ld", dividend % divisor)), NULL);
}

PRIM(pow)
{   char *endptr;
    long  base_value;
    long  exponent_value;
    long  result = 1;
 
    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&pow", "usage: $&pow base exponent");
    
    base_value = strtol(getstr(list->term), &endptr, 0);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&pow", "base must be an integer");
    
    exponent_value = strtol(getstr(list->next->term), &endptr, 0);
    
    if (endptr != NULL && *endptr != '\0')
        fail("$&pow", "exponent must be an integer");
    
    if (exponent_value < 0)
        fail("$&pow", "negative exponents not supported");
    
    for (long i = 0; i < exponent_value; i++)
        result *= base_value;
    
    return mklist(mkstr(str("%ld", result)), NULL);
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

PRIM(shl)
{   char *endptr_for_value;
    char *endptr_for_shift;
    long  input_value;
    long  shift_amount;
    const int max_shift_bits = sizeof(long) * 8 - 1;
 
    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&shl", "usage: $&shl value shift_amount");
    
    input_value = strtol(getstr(list->term), &endptr_for_value, 0);
    
    if (endptr_for_value != NULL && *endptr_for_value != '\0')
        fail("$&shl", "value argument must be an integer");
    
    shift_amount = strtol(getstr(list->next->term), &endptr_for_shift, 0);
    
    if (endptr_for_shift != NULL && *endptr_for_shift != '\0')
        fail("$&shl", "shift_amount argument must be an integer");
    
    if (shift_amount < 0)
        fail("$&shl", "shift_amount cannot be negative");
    
    if (shift_amount > max_shift_bits)
        fail("$&shl", "shift_amount too large (maximum %d bits)", max_shift_bits);
    
    return mklist(mkstr(str("%ld", input_value << shift_amount)), NULL);
}

PRIM(shr)
{   char *endptr_for_value;
    char *endptr_for_shift;
    long  input_value;
    long  shift_amount;
    const int max_shift_bits = sizeof(long) * 8 - 1;
 
    if (list == NULL || list->next == NULL || list->next->next != NULL)
        fail("$&shr", "usage: $&shr value shift_amount");
    
    input_value = strtol(getstr(list->term), &endptr_for_value, 0);
    
    if (endptr_for_value != NULL && *endptr_for_value != '\0')
        fail("$&shr", "value argument must be an integer");
    
    shift_amount = strtol(getstr(list->next->term), &endptr_for_shift, 0);
    
    if (endptr_for_shift != NULL && *endptr_for_shift != '\0')
        fail("$&shr", "shift_amount argument must be an integer");
    
    if (shift_amount < 0)
        fail("$&shr", "shift_amount cannot be negative");
    
    if (shift_amount > max_shift_bits)
        fail("$&shr", "shift_amount too large (maximum %d bits)", max_shift_bits);
    
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
    X(add);
    X(sub);
    X(mul);
    X(div);
    X(mod);
    X(pow);
    X(abs);
    X(min);
    X(max);
    X(count);
    
    /* Bitwise operations */
    X(shl);
    X(shr);
    X(and);
    X(or);
    X(xor);
    X(not);
    
    return primdict;
}
