/* syntax-infix.h -- infix operator processing for ES shell syntax parser */

#ifndef SYNTAX_INFIX_H
#define SYNTAX_INFIX_H

/* Forward declaration to avoid circular includes */
struct Tree;

/* Infix operator classification */
typedef enum {
        infixNone,
        infixAdd,
        infixSub,
        infixMul,
        infixDiv,
        infixMod,
        infixPow,
        infixMin,
        infixMax,
        /* Integer-specific arithmetic */
        infixIntAdd,
        infixIntSub,
        infixIntMul,
        infixIntDiv,
        /* Bitwise operations */
        infixBitAnd,
        infixBitOr,
        infixBitXor,
        infixShiftLeft,
        infixShiftRight,
        /* Comparison operations */
        infixGreater,
        infixLess,
        infixGreaterEqual,
        infixLessEqual,
        infixEqual,
        infixNotEqual
} InfixOp;

/* Public interface for infix processing */
extern struct Tree *rewriteinfix(struct Tree *first, struct Tree *args);

#endif /* SYNTAX_INFIX_H */