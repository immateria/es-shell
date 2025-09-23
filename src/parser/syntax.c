/* syntax.c -- abstract syntax tree re-writing rules ($Revision: 1.1.1.1 $) */

#include <ctype.h>

#include "es.h"
#include "input.h"
#include "syntax.h"
#include "token.h"

Tree errornode;
Tree *parsetree;

/* treecons -- create new tree list cell */
extern Tree *treecons(Tree *car, Tree *cdr) {
	assert(cdr == NULL || cdr->kind == nList);
	return (car == NULL) ? cdr
		: (cdr == NULL && car->kind == nList) ? car
		: mk(nList, car, cdr);
}

/* treeappend -- destructive append for tree lists */
extern Tree *treeappend(Tree *head, Tree *tail) {
	Tree *p, **prevp;
	for (p = head, prevp = &head; p != NULL; p = *(prevp = &p->CDR))
		assert(p->kind == nList || p->kind == nRedir);
	*prevp = tail;
	return head;
}

/* treeconsend -- destructive add node at end for tree lists */
extern Tree *treeconsend(Tree *head, Tree *tail) {
	if (tail == NULL) {
		assert(head == NULL || head->kind == nList || head->kind == nRedir);
		return head;
	}
	return treeappend(head, treecons(tail, NULL));
}

/* thunkify -- wrap a tree in thunk braces if it isn't already a thunk */
extern Tree *thunkify(Tree *tree) {
	Tree *t;
	for (t = tree; t != NULL && t->kind == nList && t->CDR == NULL; t = t->CAR)
		;
	return (t != NULL && t->kind == nThunk) ? tree : mk(nThunk, tree);
}

/* firstis -- check if the first word of a literal command matches a known string */
static Boolean firstis(Tree *t, const char *s) {
	if (t == NULL || t->kind != nList)
	        return FALSE;
	t = t->CAR;
	if (t == NULL || t->kind != nWord)
	        return FALSE;
	assert(t->u[0].s != NULL);
	return streq(t->u[0].s, s);
}

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

static Boolean matchany(const char *word, const char *const *choices, size_t count) {
        size_t i;

        if (word == NULL)
                return FALSE;
        for (i = 0; i < count; i++)
                if (caselessmatch(word, choices[i]))
                        return TRUE;
        return FALSE;
}

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
        static const char *const gewords[] = { "greater-equal", "greater-than-or-equal", "ge", "gte", ">=", "=>", "_ge_", "_>=" };
        static const char *const lewords[] = { "less-equal", "less-than-or-equal", "le", "lte", "<=", "=<", "_le_", "_<=" };
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

static Boolean isbitwisenot(Tree *t) {
        static const char *const notwords[] = { "~not", "bitwise-not", "bitwisenot" };

        if (t == NULL || t->kind != nWord || t->u[0].s == NULL)
                return FALSE;
        return matchany(t->u[0].s, notwords, sizeof notwords / sizeof notwords[0]);
}

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

static Boolean iscountword(Tree *t) {
        static const char *const countwords[] = { "count" };

        if (t == NULL || t->kind != nWord || t->u[0].s == NULL)
                return FALSE;
        return matchany(t->u[0].s, countwords, sizeof countwords / sizeof countwords[0]);
}

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

static Tree *makeinfixcall(InfixOp op, Tree *lhs, Tree *rhs) {
        const char *prim;
        lhs = normalizeinfix(lhs);
        rhs = normalizeinfix(rhs);
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

static Tree *parseproduct(Tree *lhs, Tree **restp, Boolean *sawop);
static Tree *parsesum(Tree *lhs, Tree **restp, Boolean *sawop);
static Tree *parsebitwise(Tree *lhs, Tree **restp, Boolean *sawop);
static Tree *parsecomparison(Tree *lhs, Tree **restp, Boolean *sawop);

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
                        return parsed;
                }
                return normalizeinfix(parsed);
        }
        return normalizeinfix(operand);
}

static Tree *parseproduct(Tree *lhs, Tree **restp, Boolean *sawop) {
        Tree *result = resolveinfixoperand(lhs);
        if (result == NULL)
                return NULL;
        while (*restp != NULL) {
                Tree *node = *restp;
                if (node->kind != nList)
                        return NULL;
                InfixOp op = classifyinfix(node->CAR);
                if (op != infixMul && op != infixDiv && op != infixMod &&
                                op != infixPow &&
                                op != infixIntMul && op != infixIntDiv &&
                                op != infixShiftLeft && op != infixShiftRight)
                        break;
                if (sawop != NULL)
                        *sawop = TRUE;
                *restp = node->CDR;
                if (*restp == NULL)
	                return NULL;
	        node = *restp;
	        if (node->kind != nList)
	                return NULL;
                Tree *rhs = resolveinfixoperand(node->CAR);
                if (rhs == NULL)
                        return NULL;
                *restp = node->CDR;
                result = makeinfixcall(op, result, rhs);
	        if (result == NULL)
	                return NULL;
	}
	return result;
}

static Tree *parsesum(Tree *lhs, Tree **restp, Boolean *sawop) {
        Tree *result = parseproduct(lhs, restp, sawop);
        if (result == NULL)
                return NULL;
        while (*restp != NULL) {
	        Tree *node = *restp;
	        InfixOp op;
	        Tree *rhsnode;
	        Tree *rhs;

	        if (node->kind != nList)
	                return NULL;
                op = classifyinfix(node->CAR);
                if (op != infixAdd && op != infixSub &&
                                op != infixIntAdd && op != infixIntSub &&
                                op != infixMin && op != infixMax)
                        break;
                if (sawop != NULL)
                        *sawop = TRUE;
                *restp = node->CDR;
                if (*restp == NULL)
                        return NULL;
                rhsnode = *restp;
                if (rhsnode->kind != nList)
	                return NULL;
	        rhs = rhsnode->CAR;
	        *restp = rhsnode->CDR;
	        rhs = parseproduct(rhs, restp, sawop);
	        if (rhs == NULL)
	                return NULL;
                result = makeinfixcall(op, result, rhs);
                if (result == NULL)
                        return NULL;
        }
        return result;
}

static Tree *parsebitwise(Tree *lhs, Tree **restp, Boolean *sawop) {
        Tree *result = parsesum(lhs, restp, sawop);
        if (result == NULL)
                return NULL;
        while (*restp != NULL) {
                Tree *node = *restp;
                InfixOp op;
                Tree *rhsnode;
                Tree *rhs;

                if (node->kind != nList)
                        return NULL;
                op = classifyinfix(node->CAR);
                if (op != infixBitAnd && op != infixBitOr && op != infixBitXor)
                        break;
                if (sawop != NULL)
                        *sawop = TRUE;
                *restp = node->CDR;
                if (*restp == NULL)
                        return NULL;
                rhsnode = *restp;
                if (rhsnode->kind != nList)
                        return NULL;
                rhs = rhsnode->CAR;
                *restp = rhsnode->CDR;
                rhs = parsesum(rhs, restp, sawop);
                if (rhs == NULL)
                        return NULL;
                result = makeinfixcall(op, result, rhs);
                if (result == NULL)
                        return NULL;
        }
        return result;
}

static Tree *parsecomparison(Tree *lhs, Tree **restp, Boolean *sawop) {
        Tree *result = parsebitwise(lhs, restp, sawop);
        if (result == NULL)
                return NULL;
        while (*restp != NULL) {
                Tree *node = *restp;
                InfixOp op;
                Tree *rhsnode;
                Tree *rhs;

                if (node->kind != nList)
                        return NULL;
                op = classifyinfix(node->CAR);
                if (op != infixGreater && op != infixLess && op != infixGreaterEqual &&
                                op != infixLessEqual && op != infixEqual && op != infixNotEqual)
                        break;
                if (sawop != NULL)
                        *sawop = TRUE;
                *restp = node->CDR;
                if (*restp == NULL)
                        return NULL;
                rhsnode = *restp;
                if (rhsnode->kind != nList)
                        return NULL;
                rhs = rhsnode->CAR;
                *restp = rhsnode->CDR;
                rhs = parsebitwise(rhs, restp, sawop);
                if (rhs == NULL)
                        return NULL;
                result = makeinfixcall(op, result, rhs);
                if (result == NULL)
                        return NULL;
        }
        return result;
}

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

/* prefix -- prefix a tree with a given word */
extern Tree *prefix(char *s, Tree *t) {
	return treecons(mk(nWord, s), t);
}

/* flatten -- flatten the output of the glommer so we can pass the result as a single element */
extern Tree *flatten(Tree *t, char *sep) {
	return mk(nCall, prefix("%flatten", treecons(mk(nQword, sep), treecons(t, NULL))));
}

/* backquote -- create a backquote command */
extern Tree *backquote(Tree *ifs, Tree *body) {
	return mk(nCall,
		  prefix("%backquote",
		  	 treecons(flatten(ifs, ""),
				  treecons(body, NULL))));
}

/* fnassign -- translate a function definition into an assignment */
extern Tree *fnassign(Tree *name, Tree *defn) {
	return mk(nAssign, mk(nConcat, mk(nWord, "fn-"), name), defn);
}

/* mklambda -- create a lambda */
extern Tree *mklambda(Tree *params, Tree *body) {
	return mk(nLambda, params, body);
}

/* mkseq -- destructively add to a sequence of nList/nThink operations */
extern Tree *mkseq(char *op, Tree *t1, Tree *t2) {
	Tree *tail;
	Boolean sametail;

	if (streq(op, "%seq")) {
		if (t1 == NULL)
			return t2;
		if (t2 == NULL)
			return t1;
	}

	sametail = firstis(t2, op);
	tail = sametail ? t2->CDR : treecons(thunkify(t2), NULL);
	if (firstis(t1, op))
		return treeappend(t1, tail);
	t1 = thunkify(t1);
	if (sametail) {
		  t2->CDR = treecons(t1, tail);
		  return t2;
	}
	return prefix(op, treecons(t1, tail));
}

/* mkpipe -- assemble a pipe from the commands that make it up (destructive) */
extern Tree *mkpipe(Tree *t1, int outfd, int infd, Tree *t2) {
	Tree *tail;
	Boolean pipetail;

	pipetail = firstis(t2, "%pipe");
	tail = prefix(pstr("%d", outfd),
		      prefix(pstr("%d", infd),
			     pipetail ? t2->CDR : treecons(thunkify(t2), NULL)));
	if (firstis(t1, "%pipe"))
		return treeappend(t1, tail);
	t1 = thunkify(t1);
	if (pipetail) {
		  t2->CDR = treecons(t1, tail);
		  return t2;
	}
	return prefix("%pipe", treecons(t1, tail));
}

/*
 * redirections -- these involve queueing up redirection in the prefix of a
 *	tree and then rewriting the tree to include the appropriate commands
 */

static Tree placeholder = { nRedir, {{NULL}} };

extern Tree *redirect(Tree *t) {
	Tree *r, *p;
	if (t == NULL)
		return NULL;
	if (t->kind != nRedir)
		return t;
	r = t->CAR;
	t = t->CDR;
	for (; r->kind == nRedir; r = r->CDR)
		t = treeappend(t, r->CAR);
	for (p = r; p->CAR != &placeholder; p = p->CDR) {
		assert(p != NULL);
		assert(p->kind == nList);
	}
	if (firstis(r, "%heredoc"))
		if (!queueheredoc(r))
			return &errornode;
	p->CAR = thunkify(redirect(t));
	return r;
}

extern Tree *mkredircmd(char *cmd, int fd) {
	return prefix(cmd, prefix(pstr("%d", fd), NULL));
}

extern Tree *mkredir(Tree *cmd, Tree *file) {
	Tree *word = NULL;
	if (file != NULL && file->kind == nThunk) {	/* /dev/fd operations */
		char *op;
		Tree *var;
		static int id = 0;
		if (firstis(cmd, "%open"))
			op = "%readfrom";
		else if (firstis(cmd, "%create"))
			op = "%writeto";
		else {
			yyerror("bad /dev/fd redirection");
			op = "";
		}
		var = mk(nWord, pstr("_devfd%d", id++));
		cmd = treecons(
			mk(nWord, op),
			treecons(var, NULL)
		);
		word = treecons(mk(nVar, var), NULL);
	} else if (!firstis(cmd, "%heredoc") && !firstis(cmd, "%here"))
		file = mk(nCall, prefix("%one", treecons(file, NULL)));
	cmd = treeappend(
		cmd,
		treecons(
			file,
			treecons(&placeholder, NULL)
		)
	);
	if (word != NULL)
		cmd = mk(nRedir, word, cmd);
	return cmd;
}

/* mkclose -- make a %close node with a placeholder */
extern Tree *mkclose(int fd) {
	return prefix("%close", prefix(pstr("%d", fd), treecons(&placeholder, NULL)));
}

/* mkdup -- make a %dup node with a placeholder */
extern Tree *mkdup(int fd0, int fd1) {
	return prefix("%dup",
		      prefix(pstr("%d", fd0),
			     prefix(pstr("%d", fd1),
				    treecons(&placeholder, NULL))));
}

/* redirappend -- destructively add to the list of redirections, before any other nodes */
extern Tree *redirappend(Tree *tree, Tree *r) {
	Tree *t, **tp;
	for (; r->kind == nRedir; r = r->CDR)
		tree = treeappend(tree, r->CAR);
	assert(r->kind == nList);
	for (t = tree, tp = &tree; t != NULL && t->kind == nRedir; t = *(tp = &t->CDR))
		;
	assert(t == NULL || t->kind == nList);
	*tp = mk(nRedir, r, t);
	return tree;
}

/* mkmatch -- rewrite match as appropriate if with ~ commands */
extern Tree *mkmatch(Tree *subj, Tree *cases) {
	const char *varname = "matchexpr";
	Tree *sass, *svar, *matches;
	/*
	 * Empty match -- with no patterns to match the subject,
	 * it's like saying {if}, which simply returns true.
	 * This avoids an unnecessary call.
	 */
	if (cases == NULL)
		return thunkify(NULL);
	/*
	 * Prevent backquote substitution in the subject from executing
	 * repeatedly by assigning it to a temporary variable and using that
	 * variable as the first argument to '~' .
	 */
	sass = treecons(mk(nAssign, mk(nWord, varname), subj), NULL);
	svar = mk(nVar, mk(nWord, varname));
	matches = NULL;
	for (; cases != NULL; cases = cases->CDR) {
		Tree *match;
		Tree *pattlist = cases->CAR->CAR;
		Tree *cmd = cases->CAR->CDR;
		if (pattlist != NULL && pattlist->kind != nList)
			pattlist = treecons(pattlist, NULL);
		match = treecons(
			thunkify(mk(nMatch, svar, pattlist)),
			treecons(cmd, NULL)
		);
		matches = treeappend(matches, match);
	}
	matches = thunkify(prefix("if", matches));
	return mk(nLocal, sass, matches);
}

/* firstprepend -- insert a command node before its arg nodes after all redirections */
extern Tree *firstprepend(Tree *first, Tree *args) {
	Tree *t, **tp;
	if (first == NULL)
		return args;
	for (t = args, tp = &args; t != NULL && t->kind == nRedir; t = *(tp = &t->CDR))
		;
	assert(t == NULL || t->kind == nList);
	*tp = treecons(first, t);
	if (args->kind == nRedir)
		return args;
	return *tp;
}
