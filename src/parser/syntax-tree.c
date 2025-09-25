/* syntax-tree.c -- tree manipulation and construction utilities for ES shell */

#include "es.h"
#include "syntax.h"

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

/* prefix -- prefix a tree with a given word */
extern Tree *prefix(char *s, Tree *t) {
	return treecons(mk(nWord, s), t);
}

/* flatten -- flatten the output of the glommer so we can pass the result as a single element */
extern Tree *flatten(Tree *t, char *sep) {
	return mk(nCall, prefix("%flatten", treecons(mk(nQword, sep), treecons(t, NULL))));
}