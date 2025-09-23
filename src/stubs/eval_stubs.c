/* eval.c -- basic evaluation stubs */

#include "es.h"

/* Evaluation depth tracking */
unsigned long evaldepth = 0, maxevaldepth = MAXmaxevaldepth;

/* eval -- basic evaluation stub */
extern List *eval(List *list, Binding *binding, int flags)
{
    fail("es:eval", "eval function not implemented");
    return NULL;
}

/* eval1 -- single term evaluation stub */
extern List *eval1(Term *term, int flags)
{
    return eval(mklist(term, NULL), NULL, flags);
}

/* pathsearch -- command lookup stub */
extern List *pathsearch(Term *term)
{
    fail("es:pathsearch", "pathsearch not implemented");
    return NULL;
}

/* forkexec -- external command execution stub */
extern List *forkexec(char *cmd, List *args, Boolean inchild)
{
    fail("es:forkexec", "forkexec not implemented");
    return NULL;
}

/* bindargs -- parameter binding stub */
extern Binding *bindargs(Tree *params, List *args, Binding *binding)
{
    fail("es:bindargs", "bindargs not implemented");
    return NULL;
}

/* walk -- tree walking stub */
extern List *walk(Tree *tree, Binding *binding, int flags)
{
    fail("es:walk", "walk not implemented");
    return NULL;
}