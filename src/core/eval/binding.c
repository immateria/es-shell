/* binding.c -- variable binding and assignment operations */

#include "es.h"
#include "eval/binding.h"

/* assign -- bind a list of values to a list of variables */
extern struct List *assign(struct Tree *varform, struct Tree *valueform0, struct Binding *binding0) {
	Ref(List *, result, NULL);

	Ref(Tree *, valueform, valueform0);
	Ref(Binding *, binding, binding0);
	Ref(List *, vars, glom(varform, binding, FALSE));

	if (vars == NULL)
		fail("es:assign", "null variable name");

	Ref(List *, values, glom(valueform, binding, TRUE));
	result = values;

	for (; vars != NULL; vars = vars->next) {
		List *value;
		Ref(char *, name, getstr(vars->term));
		if (values == NULL)
			value = NULL;
		else if (vars->next == NULL || values->next == NULL) {
			value = values;
			values = NULL;
		} else {
			value = mklist(values->term, NULL);
			values = values->next;
		}
		vardef(name, binding, value);
		RefEnd(name);
	}

	RefEnd4(values, vars, binding, valueform);
	RefReturn(result);
}

/* letbindings -- create a new Binding containing let-bound variables */
extern struct Binding *letbindings(struct Tree *defn0, struct Binding *outer0,
			    struct Binding *context0, int UNUSED evalflags) {
	Ref(Binding *, binding, outer0);
	Ref(Binding *, context, context0);
	Ref(Tree *, defn, defn0);

	for (; defn != NULL; defn = defn->u[1].p) {
		assert(defn->kind == nList);
		if (defn->u[0].p == NULL)
			continue;

		Ref(Tree *, assign, defn->u[0].p);
		assert(assign->kind == nAssign);
		Ref(List *, vars, glom(assign->u[0].p, context, FALSE));
		Ref(List *, values, glom(assign->u[1].p, context, TRUE));

		if (vars == NULL)
			fail("es:let", "null variable name");

		for (; vars != NULL; vars = vars->next) {
			List *value;
			Ref(char *, name, getstr(vars->term));
			if (values == NULL)
				value = NULL;
			else if (vars->next == NULL || values->next == NULL) {
				value = values;
				values = NULL;
			} else {
				value = mklist(values->term, NULL);
				values = values->next;
			}
			binding = mkbinding(name, value, binding);
			RefEnd(name);
		}

		RefEnd3(values, vars, assign);
	}

	RefEnd2(defn, context);
	RefReturn(binding);
}

/* localbind -- recursively convert a Bindings list into dynamic binding */
extern struct List *localbind(struct Binding *dynamic0, struct Binding *lexical0,
		       struct Tree *body0, int evalflags) {
	if (dynamic0 == NULL)
		return walk(body0, lexical0, evalflags);
	else {
		Push p;
		Ref(List *, result, NULL);
		Ref(Tree *, body, body0);
		Ref(Binding *, dynamic, dynamic0);
		Ref(Binding *, lexical, lexical0);

		varpush(&p, dynamic->name, dynamic->defn);
		result = localbind(dynamic->next, lexical, body, evalflags);
		varpop(&p);

		RefEnd3(lexical, dynamic, body);
		RefReturn(result);
	}
}
	
/* local -- build, recursively, one layer of local assignment */
extern struct List *local(struct Tree *defn, struct Tree *body0,
		   struct Binding *bindings0, int evalflags) {
	Ref(List *, result, NULL);
	Ref(Tree *, body, body0);
	Ref(Binding *, bindings, bindings0);
	Ref(Binding *, dynamic,
	    reversebindings(letbindings(defn, NULL, bindings, evalflags)));

	result = localbind(dynamic, bindings, body, evalflags);

	RefEnd3(dynamic, bindings, body);
	RefReturn(result);
}

/* bindargs -- bind an argument list to the parameters of a lambda */
extern struct Binding *bindargs(struct Tree *params, struct List *args, struct Binding *binding) {
	if (params == NULL)
		return mkbinding("*", args, binding);

	gcdisable();

	for (; params != NULL; params = params->u[1].p) {
		Tree *param;
		List *value;
		assert(params->kind == nList);
		param = params->u[0].p;
		assert(param->kind == nWord || param->kind == nQword);
		if (args == NULL)
			value = NULL;
		else if (params->u[1].p == NULL || args->next == NULL) {
			value = args;
			args = NULL;
		} else {
			value = mklist(args->term, NULL);
			args = args->next;
		}
		binding = mkbinding(param->u[0].s, value, binding);
	}

	Ref(Binding *, result, binding);
	gcenable();
	RefReturn(result);
}