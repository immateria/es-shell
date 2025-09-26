/* binding.h -- variable binding and assignment interface */

#ifndef EVAL_BINDING_H
#define EVAL_BINDING_H

/* Forward declarations - these types are defined in es.h */
struct List;
struct Tree;  
struct Binding;

/* Function pointer type for walk function to avoid circular dependencies */
typedef struct List *(*WalkFunc)(struct Tree *tree, struct Binding *binding, int flags);

/* Variable assignment - bind values to variables */
extern struct List *assign(struct Tree *varform, struct Tree *valueform, struct Binding *binding);

/* Create let-bound variable bindings */
extern struct Binding *letbindings(struct Tree *defn, struct Binding *outer, struct Binding *context, int evalflags);

/* Build local assignment layer */
extern struct List *local(struct Tree *defn, struct Tree *body, struct Binding *bindings, int evalflags, WalkFunc walk_func);

/* Recursively convert bindings list into dynamic binding */
extern struct List *localbind(struct Binding *dynamic, struct Binding *lexical, struct Tree *body, int evalflags, WalkFunc walk_func);

/* Bind arguments to lambda parameters */
extern struct Binding *bindargs(struct Tree *params, struct List *args, struct Binding *binding);

#endif /* EVAL_BINDING_H */