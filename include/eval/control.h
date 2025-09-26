/* control.h -- control flow evaluation interface */

#ifndef EVAL_CONTROL_H
#define EVAL_CONTROL_H

/* Forward declarations - these types are defined in es.h */
struct List;
struct Tree;
struct Binding;

/* For loop evaluation */
extern struct List *forloop(struct Tree *defn, struct Tree *body, struct Binding *binding, int evalflags);

/* Pattern matching - returns true/false */
extern struct List *matchpattern(struct Tree *subjectform, struct Tree *patternform, struct Binding *binding);

/* Pattern extraction - returns matched elements */
extern struct List *extractpattern(struct Tree *subjectform, struct Tree *patternform, struct Binding *binding);

#endif /* EVAL_CONTROL_H */