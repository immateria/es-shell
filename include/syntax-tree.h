/* syntax-tree.h -- tree manipulation and construction utilities for ES shell */

#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

/* Forward declaration to avoid circular includes */
struct Tree;

/* Core tree list manipulation */
extern struct Tree *treecons(struct Tree *car, struct Tree *cdr);
extern struct Tree *treeappend(struct Tree *head, struct Tree *tail);
extern struct Tree *treeconsend(struct Tree *head, struct Tree *tail);
extern struct Tree *thunkify(struct Tree *tree);

/* Tree construction utilities */
extern struct Tree *prefix(char *s, struct Tree *t);
extern struct Tree *flatten(struct Tree *t, char *sep);

#endif /* SYNTAX_TREE_H */