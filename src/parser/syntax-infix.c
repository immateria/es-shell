/* syntax-infix.c -- infix operator processing and precedence parsing for ES shell */

#include <ctype.h>

#include "es.h"
#include "syntax.h"
#include "syntax-infix.h"

/* Case-insensitive string matching utility */
static Boolean caselessmatch(const char *word, const char *keyword) {
        unsigned char cw, ck;

        if (word == NULL || keyword == NULL)
                return FALSE;
        while (*word != '\0' && *keyword != '\0') {
                cw = (unsigned char) *word++;
                ck = (unsigned char) *keyword++;
                if (tolower(cw) != tolower(ck))
                        return FALSE;
        }
        return *word == '\0' && *keyword == '\0';
}

/* Match word against array of string choices */
static Boolean matchany(const char *word, const char *const *choices, size_t count) {
        size_t i;

        if (word == NULL)
                return FALSE;
        for (i = 0; i < count; i++)
                if (caselessmatch(word, choices[i]))
                        return TRUE;
        return FALSE;
}

/* Classify a word tree node as an infix operator */
static InfixOp classifyinfix(Tree *t) {
        static const char *const addwords[] = { "plus", "+" };
        static const char *const subwords[] = { "minus", "subtract", "-", "−" };
        static const char *const mulwords[] = { "multiply", "multiplied-by", "*", "×", "times" };
        static const char *const divwords[] = { "divide", "divided-by", "/", "÷", "div" };
        static const char *const modwords[] = { "mod", "modulo", "modulus", "%" };
        static const char *const powwords[] = { "power", "pow", "raised-to", "to-the-power-of", "**" };
        static const char *const minwords[] = { "min", "minimum" };
        static const char *const maxwords[] = { "max", "maximum" };
        
        /* Integer-specific arithmetic operators */
        static const char *const intaddwords[] = { "int-plus", "int-add", "intplus", "intadd" };
        static const char *const intsubwords[] = { "int-minus", "int-subtract", "intminus", "intsubtract" };
        static const char *const intmulwords[] = { "int-multiply", "int-times", "intmultiply", "inttimes" };
        static const char *const intdivwords[] = { "int-divide", "int-div", "intdivide", "intdiv" };
        
        
        /* Bitwise operators with explicit names */
        static const char *const bandwords[] = { "bitwise-and", "bitwiseand", "∧" };
        static const char *const borwords[] = { "bitwise-or", "bitwiseor", "∨" };
        static const char *const bxorwords[] = { "bitwise-xor", "bitwisexor", "⊻" };
        static const char *const shlwords[] = { "bitwise-shift-left", "shift-left", "shift-left-by" };
        static const char *const shrwords[] = { "bitwise-shift-right", "shift-right", "shift-right-by" };
        static const char *const gtwords[] = { "greater", "greater-than", "gt", ">", "_gt_", "_>", "_greater" };
        static const char *const ltwords[] = { "less", "less-than", "lt", "<", "_lt_", "_<", "_less" };
        static const char *const gewords[] = { "greater-equal", "greater-than-or-equal", "ge", "gte", ">=", "=>", "_ge_", "_>=", "≥" };
        static const char *const lewords[] = { "less-equal", "less-than-or-equal", "le", "lte", "<=", "=<", "_le_", "_<=", "≤" };
        static const char *const eqwords[] = { "equal", "equals", "eq", "==", "===", "_eq_", "_==" };
        static const char *const newords[] = { "not-equal", "not-equals", "ne", "neq", "!=", "=/=", "_ne_", "_!=" };

        if (t == NULL || t->kind != nWord || t->u[0].s == NULL)
                return infixNone;
        const char *s = t->u[0].s;
        if (matchany(s, addwords, sizeof addwords / sizeof addwords[0]))
                return infixAdd;
        if (matchany(s, subwords, sizeof subwords / sizeof subwords[0]))
                return infixSub;
        if (matchany(s, mulwords, sizeof mulwords / sizeof mulwords[0]))
                return infixMul;
        if (matchany(s, divwords, sizeof divwords / sizeof divwords[0]))
                return infixDiv;
        if (matchany(s, modwords, sizeof modwords / sizeof modwords[0]))
                return infixMod;
        if (matchany(s, powwords, sizeof powwords / sizeof powwords[0]))
                return infixPow;
        if (matchany(s, minwords, sizeof minwords / sizeof minwords[0]))
                return infixMin;
        if (matchany(s, maxwords, sizeof maxwords / sizeof maxwords[0]))
                return infixMax;
        /* Integer-specific arithmetic */
        if (matchany(s, intaddwords, sizeof intaddwords / sizeof intaddwords[0]))
                return infixIntAdd;
        if (matchany(s, intsubwords, sizeof intsubwords / sizeof intsubwords[0]))
                return infixIntSub;
        if (matchany(s, intmulwords, sizeof intmulwords / sizeof intmulwords[0]))
                return infixIntMul;
        if (matchany(s, intdivwords, sizeof intdivwords / sizeof intdivwords[0]))
                return infixIntDiv;
        /* (Type conversion operators are unary, not infix) */
        if (matchany(s, bandwords, sizeof bandwords / sizeof bandwords[0]))
                return infixBitAnd;
        if (matchany(s, borwords, sizeof borwords / sizeof borwords[0]))
                return infixBitOr;
        if (matchany(s, bxorwords, sizeof bxorwords / sizeof bxorwords[0]))
                return infixBitXor;
        if (matchany(s, shlwords, sizeof shlwords / sizeof shlwords[0]))
                return infixShiftLeft;
        if (matchany(s, shrwords, sizeof shrwords / sizeof shrwords[0]))
                return infixShiftRight;
        if (matchany(s, gtwords, sizeof gtwords / sizeof gtwords[0]))
                return infixGreater;
        if (matchany(s, ltwords, sizeof ltwords / sizeof ltwords[0]))
                return infixLess;
        if (matchany(s, gewords, sizeof gewords / sizeof gewords[0]))
                return infixGreaterEqual;
        if (matchany(s, lewords, sizeof lewords / sizeof lewords[0]))
                return infixLessEqual;
        if (matchany(s, eqwords, sizeof eqwords / sizeof eqwords[0]))
                return infixEqual;
        if (matchany(s, newords, sizeof newords / sizeof newords[0]))
                return infixNotEqual;
        return infixNone;
}

/* Check if tree represents a bitwise NOT operator */
static Boolean isbitwisenot(Tree *t) {
        static const char *const notwords[] = { "~not", "bitwise-not", "bitwisenot" };

        if (t == NULL || t->kind != nWord || t->u[0].s == NULL)
                return FALSE;
        return matchany(t->u[0].s, notwords, sizeof notwords / sizeof notwords[0]);
}

/* Check if tree represents an absolute value operator */
static Boolean isabsword(Tree *t) {
        static const char *const abswords[] = {
                "abs",
                "absolute",
                "absolute-value",
                "absolute-value-of"
        };

        if (t == NULL || t->kind != nWord || t->u[0].s == NULL)
                return FALSE;
        return matchany(t->u[0].s, abswords, sizeof abswords / sizeof abswords[0]);
}

/* Check if tree represents a count operator */
static Boolean iscountword(Tree *t) {
        static const char *const countwords[] = { "count" };

        if (t == NULL || t->kind != nWord || t->u[0].s == NULL)
                return FALSE;
        return matchany(t->u[0].s, countwords, sizeof countwords / sizeof countwords[0]);
}

/* Normalize operands for infix operations */
static Tree *normalizeinfix(Tree *operand) {
	if (operand != NULL && operand->kind == nWord) {
	        char *end;
	        strtol(operand->u[0].s, &end, 10);
	        /* Keep numbers as words, and keep other literals as words too */
	        /* Don't auto-convert to variables - this caused the same issue as arithword */
	        return operand;
	}
	return operand;
}

/* Convert infix operation to primitive function call */
static Tree *makeinfixcall(InfixOp op, Tree *lhs, Tree *rhs) {
        const char *prim;
        /* For infix operations, convert non-numeric words to variables */
        if (lhs != NULL && lhs->kind == nWord) {
                char *end;
                /* Check if it's an integer */
                strtol(lhs->u[0].s, &end, 10);
                if (*end != '\0') {
                        /* Check if it's a floating-point number */
                        strtod(lhs->u[0].s, &end);
                        if (*end != '\0')
                                lhs = mk(nVar, lhs);
                }
        }
        if (rhs != NULL && rhs->kind == nWord) {
                char *end;
                /* Check if it's an integer */
                strtol(rhs->u[0].s, &end, 10);
                if (*end != '\0') {
                        /* Check if it's a floating-point number */
                        strtod(rhs->u[0].s, &end);
                        if (*end != '\0')
                                rhs = mk(nVar, rhs);
                }
        }
        switch (op) {
        case infixAdd:
                prim = "%addition";
                break;
        case infixSub:
                prim = "%subtraction";
                break;
        case infixMul:
                prim = "%multiplication";
                break;
        case infixDiv:
                prim = "%division";
                break;
        case infixMod:
                prim = "%modulo";
                break;
        case infixPow:
                prim = "%pow";
                break;
        case infixMin:
                prim = "%min";
                break;
        case infixMax:
                prim = "%max";
                break;
        /* Integer-specific arithmetic */
        case infixIntAdd:
                prim = "%intaddition";
                break;
        case infixIntSub:
                prim = "%intsubtraction";
                break;
        case infixIntMul:
                prim = "%intmultiplication";
                break;
        case infixIntDiv:
                prim = "%intdivision";
                break;
        /* (Type conversion cases removed - they are unary operators) */
        case infixBitAnd:
                prim = "%bitwiseand";
                break;
        case infixBitOr:
                prim = "%bitwiseor";
                break;
        case infixBitXor:
                prim = "%bitwisexor";
                break;
        case infixShiftLeft:
                prim = "%bitwiseshiftleft";
                break;
        case infixShiftRight:
                prim = "%bitwiseshiftright";
                break;
        case infixGreater:
                prim = "%greater";
                break;
        case infixLess:
                prim = "%less";
                break;
        case infixGreaterEqual:
                prim = "%greaterequal";
                break;
        case infixLessEqual:
                prim = "%lessequal";
                break;
        case infixEqual:
                prim = "%equal";
                break;
        case infixNotEqual:
                prim = "%notequal";
                break;
        default:
                return NULL;
        }
        return mk(nCall, prefix((char *) prim,
                treecons(lhs, treecons(rhs, NULL))));
}

/* Forward declarations for precedence parsing */
static Tree *parseproduct(Tree *lhs, Tree **restp, Boolean *sawop);
static Tree *parsesum(Tree *lhs, Tree **restp, Boolean *sawop);
static Tree *parsebitwise(Tree *lhs, Tree **restp, Boolean *sawop);
static Tree *parsecomparison(Tree *lhs, Tree **restp, Boolean *sawop);

/* Resolve infix operand, handling grouped expressions */
static Tree *resolveinfixoperand(Tree *operand) {
        if (operand == NULL)
                return NULL;
        if (operand->kind == nList) {
                Tree *group = operand;
                if (operand->CAR != NULL && operand->CAR->kind == nList && operand->CDR == NULL)
                        group = operand->CAR;
                if (group == NULL || group->kind != nList)
                        return NULL;
                Tree *rest = group->CDR;
                Boolean localsaw = FALSE;
                Tree *parsed = parsecomparison(group->CAR, &rest, &localsaw);
                if (parsed == NULL || rest != NULL)
                        return NULL;
                if (localsaw) {
                        if (parsed->kind != nCall)
                                return NULL;
                        return parsed->u[0].p;
                }
                return parsed;
        }
        return normalizeinfix(operand);
}

/* Parse multiplication, division, modulo (highest precedence) */
static Tree *parseproduct(Tree *lhs, Tree **restp, Boolean *sawop) {
        Tree *rest = *restp;

        while (rest != NULL) {
                InfixOp op = classifyinfix(rest->CAR);
                Tree *rhs;

                if (op != infixMul && op != infixDiv && op != infixMod && op != infixPow && 
                    op != infixMin && op != infixMax && 
                    op != infixIntMul && op != infixIntDiv)
                        break;
                if (rest->CDR == NULL)
                        return NULL;
                rest = rest->CDR;
                rhs = resolveinfixoperand(rest->CAR);
                if (rhs == NULL)
                        return NULL;
                lhs = makeinfixcall(op, lhs, rhs);
                if (lhs == NULL)
                        return NULL;
                *sawop = TRUE;
                rest = rest->CDR;
        }
        *restp = rest;
        return lhs;
}

/* Parse addition and subtraction */
static Tree *parsesum(Tree *lhs, Tree **restp, Boolean *sawop) {
        Tree *rest = *restp;

        lhs = parseproduct(lhs, &rest, sawop);
        if (lhs == NULL)
                return NULL;
        while (rest != NULL) {
                InfixOp op = classifyinfix(rest->CAR);
                Tree *rhs;

                if (op != infixAdd && op != infixSub && op != infixIntAdd && op != infixIntSub)
                        break;
                if (rest->CDR == NULL)
                        return NULL;
                rest = rest->CDR;
                rhs = resolveinfixoperand(rest->CAR);
                if (rhs == NULL)
                        return NULL;
                rest = rest->CDR;  /* Advance past the operand we just consumed */
                rhs = parseproduct(rhs, &rest, sawop);
                if (rhs == NULL)
                        return NULL;
                lhs = makeinfixcall(op, lhs, rhs);
                if (lhs == NULL)
                        return NULL;
                *sawop = TRUE;
        }
        *restp = rest;
        return lhs;
}

/* Parse bitwise operations */
static Tree *parsebitwise(Tree *lhs, Tree **restp, Boolean *sawop) {
        Tree *rest = *restp;

        lhs = parsesum(lhs, &rest, sawop);
        if (lhs == NULL)
                return NULL;
        while (rest != NULL) {
                InfixOp op = classifyinfix(rest->CAR);
                Tree *rhs;

                if (op != infixBitAnd && op != infixBitOr && op != infixBitXor && 
                    op != infixShiftLeft && op != infixShiftRight)
                        break;
                if (rest->CDR == NULL)
                        return NULL;
                rest = rest->CDR;
                rhs = resolveinfixoperand(rest->CAR);
                if (rhs == NULL)
                        return NULL;
                rest = rest->CDR;  /* Advance past the operand we just consumed */
                rhs = parsesum(rhs, &rest, sawop);
                if (rhs == NULL)
                        return NULL;
                lhs = makeinfixcall(op, lhs, rhs);
                if (lhs == NULL)
                        return NULL;
                *sawop = TRUE;
        }
        *restp = rest;
        return lhs;
}

/* Parse comparison operations (lowest precedence) */
static Tree *parsecomparison(Tree *lhs, Tree **restp, Boolean *sawop) {
        Tree *rest = *restp;

        lhs = parsebitwise(lhs, &rest, sawop);
        if (lhs == NULL)
                return NULL;
        while (rest != NULL) {
                InfixOp op = classifyinfix(rest->CAR);
                Tree *rhs;

                if (op != infixGreater && op != infixLess && op != infixGreaterEqual && 
                    op != infixLessEqual && op != infixEqual && op != infixNotEqual)
                        break;
                if (rest->CDR == NULL)
                        return NULL;
                rest = rest->CDR;
                rhs = resolveinfixoperand(rest->CAR);
                if (rhs == NULL)
                        return NULL;
                rest = rest->CDR;  /* Advance past the operand we just consumed */
                rhs = parsebitwise(rhs, &rest, sawop);
                if (rhs == NULL)
                        return NULL;
                lhs = makeinfixcall(op, lhs, rhs);
                if (lhs == NULL)
                        return NULL;
                *sawop = TRUE;
        }
        *restp = rest;
        return lhs;
}

/* Main entry point for infix expression rewriting */
extern Tree *rewriteinfix(Tree *first, Tree *args) {
        Tree *rest;
        Tree *result;
        Boolean sawop = FALSE;

        if (first == NULL)
                return NULL;
        if (isbitwisenot(first)) {
                Tree *operand;

                if (args == NULL)
                        return NULL;
                rest = args->CDR;
                operand = parsecomparison(args->CAR, &rest, &sawop);
                if (operand == NULL || rest != NULL)
                        return NULL;
                operand = normalizeinfix(operand);
                result = mk(nCall, prefix("%bitwisenot", treecons(operand, NULL)));
                if (result == NULL)
                        return NULL;
                return result->u[0].p;
        }
        if (isabsword(first)) {
                Tree *operand;

                if (args == NULL || args->CDR != NULL)
                        return NULL;
                operand = resolveinfixoperand(args->CAR);
                if (operand == NULL)
                        return NULL;
                result = mk(nCall, prefix("%abs", treecons(operand, NULL)));
                if (result == NULL)
                        return NULL;
                return result->u[0].p;
        }
        if (iscountword(first)) {
                Tree *converted = NULL;
                Tree **tailp = &converted;
                Tree *argnode;

                for (argnode = args; argnode != NULL; argnode = argnode->CDR) {
                        Tree *operand = resolveinfixoperand(argnode->CAR);
                        Tree *cell;

                        if (operand == NULL)
                                return NULL;
                        cell = treecons(operand, NULL);
                        if (cell == NULL)
                                return NULL;
                        *tailp = cell;
                        tailp = &cell->CDR;
                }
                result = mk(nCall, prefix("%count", converted));
                if (result == NULL)
                        return NULL;
                return result->u[0].p;
        }
        rest = args;
        result = parsecomparison(first, &rest, &sawop);
        if (!sawop || result == NULL || rest != NULL)
                return NULL;
        if (result->kind != nCall)
                return NULL;
        return result->u[0].p;
}