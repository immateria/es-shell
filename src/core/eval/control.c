/* control.c -- control flow evaluation operations */

#include "es.h"
#include "eval/control.h"

/* forloop -- evaluate a for loop */
extern List *forloop(Tree *defn0, Tree *body0,
		     Binding *binding, int evalflags) {
	static List MULTIPLE = { NULL, NULL };

	Ref(List *, result, ltrue);
	Ref(Binding *, outer, binding);
	Ref(Binding *, looping, NULL);
	Ref(Tree *, body, body0);

	Ref(Tree *, defn, defn0);
	for (; defn != NULL; defn = defn->u[1].p) {
		assert(defn->kind == nList);
		if (defn->u[0].p == NULL)
			continue;
		Ref(Tree *, assign, defn->u[0].p);
		assert(assign->kind == nAssign);
		Ref(List *, vars, glom(assign->u[0].p, outer, FALSE));
		Ref(List *, list, glom(assign->u[1].p, outer, TRUE));
		if (vars == NULL)
			fail("es:for", "null variable name");
		for (; vars != NULL; vars = vars->next) {
			char *var = getstr(vars->term);
			looping = mkbinding(var, list, looping);
			list = &MULTIPLE;
		}
		RefEnd3(list, vars, assign);
		SIGCHK();
	}
	looping = reversebindings(looping);
	RefEnd(defn);

	ExceptionHandler

		for (;;) {
			Boolean allnull = TRUE;
			Ref(Binding *, bp, outer);
			Ref(Binding *, lp, looping);
			Ref(Binding *, sequence, NULL);
			for (; lp != NULL; lp = lp->next) {
				Ref(List *, value, NULL);
				if (lp->defn != &MULTIPLE)
					sequence = lp;
				assert(sequence != NULL);
				if (sequence->defn != NULL) {
					value = mklist(sequence->defn->term,
						       NULL);
					sequence->defn = sequence->defn->next;
					allnull = FALSE;
				}
				bp = mkbinding(lp->name, value, bp);
				RefEnd(value);
			}
			RefEnd2(sequence, lp);
			if (allnull) {
				RefPop(bp);
				break;
			}
			result = walk(body, bp, evalflags & eval_exitonfalse);
			RefEnd(bp);
			SIGCHK();
		}

	CatchException (e)

		if (!termeq(e->term, "break"))
			throw(e);
		result = e->next;

	EndExceptionHandler

	RefEnd3(body, looping, outer);
	RefReturn(result);
}

/* matchpattern -- does the text match a pattern? */
extern List *matchpattern(Tree *subjectform0, Tree *patternform0,
			  Binding *binding) {
	Boolean result;
	List *pattern;
	Ref(Binding *, bp, binding);
	Ref(Tree *, patternform, patternform0);
	Ref(List *, subject, glom(subjectform0, bp, TRUE));
	Ref(StrList *, quote, NULL);
	pattern = glom2(patternform, bp, &quote);
	result = listmatch(subject, pattern, quote);
	RefEnd4(quote, subject, patternform, bp);
	return result ? ltrue : lfalse;
}

/* extractpattern -- Like matchpattern, but returns matches */
extern List *extractpattern(Tree *subjectform0, Tree *patternform0,
			    Binding *binding) {
	List *pattern;
	Ref(List *, result, NULL);
	Ref(Binding *, bp, binding);
	Ref(Tree *, patternform, patternform0);
	Ref(List *, subject, glom(subjectform0, bp, TRUE));
	Ref(StrList *, quote, NULL);
	pattern = glom2(patternform, bp, &quote);
	result = (List *) extractmatches(subject, pattern, quote);
	RefEnd4(quote, subject, patternform, bp);
	RefReturn(result);
}