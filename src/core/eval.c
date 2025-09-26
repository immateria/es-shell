/* eval.c -- evaluation of lists and trees ($Revision: 1.2 $) */

#include "es.h"
#include "eval/arithmetic.h"
#include "eval/binding.h"
#include "eval/control.h"
#include "debug.h"

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
		return local(tree->u[0].p, tree->u[1].p, binding, flags, walk);

	    case nFor:
		return forloop(tree->u[0].p, tree->u[1].p, binding, flags);
	
	    case nMatch:
		return matchpattern(tree->u[0].p, tree->u[1].p, binding);

	    case nExtract: {
		List *result = extractpattern(tree->u[0].p, tree->u[1].p, binding);
		if (result != NULL) {
			/* Print the extracted matches to stdout */
			print("%L\n", result, " ");
		}
		return result;
	    }

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

	if (++evaldepth >= maxevaldepth)
		fail("es:eval", "max-eval-depth exceeded");

	Ref(List *, list, list0);
	Ref(Binding *, binding, binding0);
	Ref(char *, funcname, NULL);

restart:
	SIGCHK();
	if (list == NULL) {
		RefPop3(funcname, binding, list);
		--evaldepth;
		return ltrue;
	}
	assert(list->term != NULL);

	if ((cp = getclosure(list->term)) != NULL) {
		switch (cp->tree->kind) {
		    case nPrim:
			assert(cp->binding == NULL);
			DEBUG_TRACE(DEBUG_PRIM, "eval: calling primitive '%s'", cp->tree->u[0].s);
			list = prim(cp->tree->u[0].s, list->next, binding, flags);
			DEBUG_TRACE(DEBUG_PRIM, "eval: primitive '%s' returned %p", cp->tree->u[0].s, (void*)list);
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
	
	/* Check for arithmetic expressions (compound like "5+3" or literals like "42") */
	if (is_arithmetic_expression(name)) {
		List *arith_result = parse_arithmetic_expression(name, list->next);
		if (arith_result != NULL) {
			/* Check if this is a literal number (single term, same as original name) */
			if (arith_result->next == list->next && 
			    arith_result->term != NULL &&
			    strcmp(getstr(arith_result->term), name) == 0) {
				/* It's a literal - return directly to avoid infinite loop */
				list = arith_result;
				RefPop(name);
				goto done;
			} else {
				/* It's a compound expression - restart evaluation */
				list = arith_result;
				RefPop(name);
				goto restart;
			}
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
	DEBUG_TRACE(DEBUG_EVAL, "eval: exiting with list=%p", (void*)list);
	DEBUG_PRINT_LIST(DEBUG_EVAL, list, "eval exit list:");
	--evaldepth;
	if ((flags & eval_exitonfalse) && !istrue(list))
		esexit(exitstatus(list));
	RefEnd2(funcname, binding);
	DEBUG_TRACE(DEBUG_EVAL, "eval: about to RefReturn list=%p", (void*)list);
	RefReturn(list);
}

/* eval1 -- evaluate a term, producing a list */
extern List *eval1(Term *term, int flags) {
	return eval(mklist(term, NULL), NULL, flags);
}