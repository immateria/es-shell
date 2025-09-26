/* eval.c -- evaluation of lists and trees ($Revision: 1.2 $) */

#include "es.h"
#include "debug.h"
#include "eval/binding.h"
#include "eval/control.h"

unsigned long evaldepth = 0, maxevaldepth = MAXmaxevaldepth;

static Noreturn failexec(char *file, List *args) {
	List *fn;
	assert(gcisblocked());
	fn = varlookup("fn-%exec-failure", NULL);
	if (fn != NULL) {
		int olderror = errno;
		Ref(List *, list, append(fn, mklist(mkstr(file), args)));
		RefAdd(file);
		gcenable();
		RefRemove(file);
		eval(list, NULL, 0);
		RefEnd(list);
		errno = olderror;
	}
	eprint("%s: %s\n", file, esstrerror(errno));
	esexit(1);
}

/* forkexec -- fork (if necessary) and exec */
extern List *forkexec(char *file, List *list, Boolean inchild) {
	int pid, status;
	Vector *env;
	gcdisable();
	env = mkenv();
	pid = efork(!inchild, FALSE);
	if (pid == 0) {
		execve(file, vectorize(list)->vector, env->vector);
		failexec(file, list);
	}
	gcenable();
	status = ewaitfor(pid);
	if ((status & 0xff) == 0) {
		sigint_newline = FALSE;
		SIGCHK();
		sigint_newline = TRUE;
	} else
		SIGCHK();
	printstatus(0, status);
	return mklist(mkterm(mkstatus(status), NULL), NULL);
}

/* walk -- walk through a tree, evaluating nodes */
extern List *walk(Tree *tree0, Binding *binding0, int flags) {
	Tree *volatile tree = tree0;
	Binding *volatile binding = binding0;

	SIGCHK();

top:
	if (tree == NULL)
		return ltrue;

	switch (tree->kind) {

	    case nConcat: case nList: case nQword: case nVar: case nVarsub:
	    case nWord: case nThunk: case nLambda: case nCall: case nPrim: {
		List *list;
		Ref(Binding *, bp, binding);
		list = glom(tree, binding, TRUE);
		binding = bp;
		RefEnd(bp);
		return eval(list, binding, flags);
	    }

	    case nAssign:
		return assign(tree->u[0].p, tree->u[1].p, binding);

	    case nLet: case nClosure:
		Ref(Tree *, body, tree->u[1].p);
		binding = letbindings(tree->u[0].p, binding, binding, flags);
		tree = body;
		RefEnd(body);
		goto top;

	    case nLocal:
		return local(tree->u[0].p, tree->u[1].p, binding, flags);

	    case nFor:
		return forloop(tree->u[0].p, tree->u[1].p, binding, flags);
	
	    case nMatch:
		return matchpattern(tree->u[0].p, tree->u[1].p, binding);

	    case nExtract:
		return extractpattern(tree->u[0].p, tree->u[1].p, binding);

	    default:
		panic("walk: bad node kind %d", tree->kind);

	}
	NOTREACHED;
}

/* pathsearch -- evaluate fn %pathsearch + some argument */
extern List *pathsearch(Term *term) {
	List *list;
	Ref(List *, search, NULL);
	search = varlookup("fn-%pathsearch", NULL);
	if (search == NULL)
		fail("es:pathsearch", "%E: fn %%pathsearch undefined", term);
	list = mklist(term, NULL);
	list = append(search, list);
	RefEnd(search);
	return eval(list, NULL, 0);
}

/* eval -- evaluate a list, producing a list */
extern List *eval(List *list0, Binding *binding0, int flags) {
	Closure *volatile cp;
	List *fn;

	DEBUG_TRACE_ENTER(DEBUG_EVAL, "eval called with depth=%lu, flags=%d", evaldepth, flags);
	DEBUG_PRINT_LIST(DEBUG_EVAL, list0, "Input list");
	DEBUG_PERF_START("eval");

	if (++evaldepth >= maxevaldepth) {
		DEBUG_TRACE(DEBUG_EVAL, "Max eval depth exceeded: %lu >= %lu", evaldepth, maxevaldepth);
		fail("es:eval", "max-eval-depth exceeded");
	}

	Ref(List *, list, list0);
	Ref(Binding *, binding, binding0);
	Ref(char *, funcname, NULL);

restart:
	SIGCHK();
	DEBUG_CHECKPOINT();  /* Interactive debugging checkpoint */
	
	if (list == NULL) {
		DEBUG_TRACE(DEBUG_EVAL, "eval: empty list, returning ltrue");
		DEBUG_PERF_END("eval");
		RefPop3(funcname, binding, list);
		--evaldepth;
		DEBUG_TRACE_EXIT(DEBUG_EVAL, "eval returning ltrue, depth=%lu", evaldepth);
		return ltrue;
	}
	assert(list->term != NULL);

	if ((cp = getclosure(list->term)) != NULL) {
		DEBUG_TRACE(DEBUG_EVAL, "eval: found closure, kind=%d", cp->tree->kind);
		switch (cp->tree->kind) {
		    case nPrim:
			assert(cp->binding == NULL);
			list = prim(cp->tree->u[0].s, list->next, binding, flags);
			break;
		    case nThunk:
			list = walk(cp->tree->u[0].p, cp->binding, flags);
			break;
		    case nLambda:
			ExceptionHandler

				Push p;
				Ref(Tree *, tree, cp->tree);
				Ref(Binding *, context,
					       bindargs(tree->u[0].p,
							list->next,
							cp->binding));
				if (funcname != NULL)
					varpush(&p, "0",
						    mklist(mkterm(funcname,
								  NULL),
							   NULL));
				list = walk(tree->u[1].p, context, flags);
				if (funcname != NULL)
					varpop(&p);
				RefEnd2(context, tree);
	
			CatchException (e)

				if (termeq(e->term, "return")) {
					list = e->next;
					goto done;
				}
				throw(e);

			EndExceptionHandler
			break;
		    case nList: {
			Ref(List *, lp, glom(cp->tree, cp->binding, TRUE));
			list = append(lp, list->next);
			RefEnd(lp);
			goto restart;
		    }
		    case nConcat: {
			Ref(Tree *, t, cp->tree);
			while (t->kind == nConcat)
				t = t->u[0].p;
			if (t->kind == nPrim)
				fail("es:eval", "invalid primitive name: %T", cp->tree);
			RefEnd(t);
		    }
		    FALLTHROUGH;
		    default:
			panic("eval: bad closure node kind %d",
			      cp->tree->kind);
		    }
		goto done;
	}

	/* the logic here is duplicated in $&whatis */

	Ref(char *, name, getstr(list->term));
	
	/* Check for compound arithmetic expression like "5+0" and parse it */
	char *op_pos = NULL;
	char *prim_name = NULL;
	
	/* Only process compound expressions: digit + operator + digit */
	size_t name_len = strlen(name);
	Boolean has_digit_op_digit = FALSE;
	for (size_t i = 1; i < name_len - 1; i++) {
		if ((name[i] == '+' || name[i] == '-' || name[i] == '*' || name[i] == '/') &&
		    isdigit(name[i-1]) && isdigit(name[i+1])) {
			has_digit_op_digit = TRUE;
			break;
		}
	}
	
	if (has_digit_op_digit) {
		
		if ((op_pos = strchr(name, '+')) != NULL) {
			prim_name = "%addition";
		} else if ((op_pos = strchr(name, '-')) != NULL) {
			/* Make sure it's not a negative number at the start */
			if (op_pos != name) {
				prim_name = "%subtraction";
			}
		} else if ((op_pos = strchr(name, '*')) != NULL) {
			prim_name = "%multiplication";
		} else if ((op_pos = strchr(name, '/')) != NULL) {
			prim_name = "%division";
		}
	}
	
	if (op_pos != NULL && prim_name != NULL && op_pos != name && *(op_pos + 1) != '\0') {
		size_t left_len = op_pos - name;
		char *left_operand = ealloc(left_len + 1);
		memcpy(left_operand, name, left_len);
		left_operand[left_len] = '\0';
		
		char *right_operand = str("%s", op_pos + 1);
		
		/* Create arithmetic call: primitive left_operand right_operand */
		List *arith_call = mklist(mkstr(prim_name), 
		                        mklist(mkstr(left_operand), 
		                              mklist(mkstr(right_operand), list->next)));
		list = arith_call;
		RefPop(name);
		goto restart;
	}
	
	/* Check for literal numbers like "5", "-3.14", "0" */
	{
		char *endptr;
		double parsed_value = strtod(name, &endptr);
		(void)parsed_value; /* Suppress unused variable warning */
		
		/* If strtod consumed the entire string, it's a valid number */
		if (*endptr == '\0' && endptr != name) {
			/* Return the number as a literal string value */
			list = mklist(mkstr(name), list->next);
			RefPop(name);
			goto done;
		}
	}
	
	fn = varlookup2("fn-", name, binding);
	if (fn != NULL) {
		funcname = name;
		list = append(fn, list->next);
		RefPop(name);
		goto restart;
	}
	if (isabsolute(name)) {
		char *error = checkexecutable(name);
		if (error != NULL)
			fail("$&whatis", "%s: %s", name, error);
		if (funcname != NULL) {
			Term *fn = mkstr(funcname);
			list = mklist(fn, list->next);
		}
		list = forkexec(name, list, flags & eval_inchild);
		RefPop(name);
		goto done;
	}
	RefEnd(name);

	fn = pathsearch(list->term);
	if (fn != NULL && fn->next == NULL
	    && (cp = getclosure(fn->term)) == NULL) {
		char *name = getstr(fn->term);
		list = forkexec(name, list, flags & eval_inchild);
		goto done;
	}

	if (fn != NULL)
		funcname = getstr(list->term);
	list = append(fn, list->next);
	goto restart;

done:
	DEBUG_PERF_END("eval");
	--evaldepth;
	DEBUG_TRACE_EXIT(DEBUG_EVAL, "eval returning result, depth=%lu", evaldepth);
	DEBUG_PRINT_LIST(DEBUG_EVAL, list, "Result list");
	
	if ((flags & eval_exitonfalse) && !istrue(list))
		esexit(exitstatus(list));
	RefEnd2(funcname, binding);
	RefReturn(list);
}

/* eval1 -- evaluate a term, producing a list */
extern List *eval1(Term *term, int flags) {
	return eval(mklist(term, NULL), NULL, flags);
}