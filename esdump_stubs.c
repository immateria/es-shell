/* esdump_stubs.c -- stub implementations for esdump only */

#include "es.h"

/* These symbols are needed by esdump but will be provided by 
   the generated initial.c in the final es binary */

unsigned long evaldepth = 0;
unsigned long maxevaldepth = 500;

List *eval(List *list, Binding *binding, int flags) {
    /* This should not be called in esdump context */
    panic("eval called in esdump - this should not happen");
    return NULL;
}

List *eval1(Term *term, int flags) {
    /* This should not be called in esdump context */
    panic("eval1 called in esdump - this should not happen");
    return NULL;
}

Binding *bindargs(Tree *params, List *args, Binding *binding) {
    /* This should not be called in esdump context */
    panic("bindargs called in esdump - this should not happen");
    return NULL;
}

List *forkexec(char *file, List *list, Boolean inchild) {
    /* This should not be called in esdump context */
    panic("forkexec called in esdump - this should not happen");
    return NULL;
}

List *pathsearch(Term *term) {
    /* This should not be called in esdump context */
    panic("pathsearch called in esdump - this should not happen");
    return NULL;
}

List *walk(Tree *tree, Binding *binding, int flags) {
    /* This should not be called in esdump context */
    panic("walk called in esdump - this should not happen");
    return NULL;
}