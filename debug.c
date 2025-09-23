/* debug.c -- Debug utility functions for es-shell */

#include "es.h"
#include "debug.h"
#include "term.h"

#ifdef ES_DEBUG

/* Global debug flags variable */
int es_debug_flags = DEBUG_NONE;

/* Initialize debug system */
void debug_init(void) {
    char *debug_env;
    fprintf(stderr, "DEBUG_INIT: Initializing debug system\n");
    
    /* Check environment variable for debug flags */
    debug_env = getenv("ES_DEBUG");
    fprintf(stderr, "DEBUG_INIT: ES_DEBUG env var = %s\n", debug_env ? debug_env : "(null)");
    if (debug_env != NULL) {
        if (strcmp(debug_env, "all") == 0) {
            es_debug_flags = DEBUG_ALL;
        } else if (strcmp(debug_env, "parse") == 0) {
            es_debug_flags = DEBUG_PARSE;
        } else if (strcmp(debug_env, "eval") == 0) {
            es_debug_flags = DEBUG_EVAL;
        } else if (strcmp(debug_env, "prim") == 0) {
            es_debug_flags = DEBUG_PRIM;
        } else if (strcmp(debug_env, "func") == 0) {
            es_debug_flags = DEBUG_FUNC;
        } else if (strcmp(debug_env, "list") == 0) {
            es_debug_flags = DEBUG_LIST;
        } else if (strcmp(debug_env, "tree") == 0) {
            es_debug_flags = DEBUG_TREE;
        } else if (strcmp(debug_env, "var") == 0) {
            es_debug_flags = DEBUG_VAR;
        } else {
            /* Try to parse as numeric flags */
            es_debug_flags = atoi(debug_env);
        }
    }
    
    if (es_debug_flags != DEBUG_NONE) {
        fprintf(stderr, "DEBUG: es-shell debug tracing enabled (flags=0x%04x)\n", es_debug_flags);
    }
}

/* Set debug flags programmatically */
void debug_set_flags(int flags) {
    es_debug_flags = flags;
}

/* Print a list structure for debugging */
void debug_print_list(List *list, const char *prefix, const char *func, int line) {
    List *lp;
    int count = 0;
    
    fprintf(stderr, "[%s:%d] %s List contents:\n", func, line, prefix);
    
    if (list == NULL) {
        fprintf(stderr, "[%s:%d]   (null list)\n", func, line);
        return;
    }
    
    for (lp = list; lp != NULL; lp = lp->next) {
        fprintf(stderr, "[%s:%d]   [%d]: ", func, line, count);
        
        if (lp->term == NULL) {
            fprintf(stderr, "(null term)");
        } else if (lp->term->str == NULL) {
            fprintf(stderr, "(null string)");
        } else {
            fprintf(stderr, "\"%s\"", lp->term->str);
        }
        
        if (lp->next != NULL) {
            fprintf(stderr, " ->");
        }
        fprintf(stderr, "\n");
        count++;
    }
    
    fprintf(stderr, "[%s:%d]   Total: %d items\n", func, line, count);
    fflush(stderr);
}

/* Print a tree structure for debugging */  
void debug_print_tree(Tree *tree, const char *prefix, const char *func, int line) {
    fprintf(stderr, "[%s:%d] %s Tree:\n", func, line, prefix);
    
    if (tree == NULL) {
        fprintf(stderr, "[%s:%d]   (null tree)\n", func, line);
        return;
    }
    
    fprintf(stderr, "[%s:%d]   kind: %d\n", func, line, tree->kind);
    
    switch (tree->kind) {
        case nWord:
            fprintf(stderr, "[%s:%d]   word: \"%s\"\n", func, line, 
                   tree->u[0].s ? tree->u[0].s : "(null)");
            break;
            
        case nQword:
            fprintf(stderr, "[%s:%d]   qword: \"%s\"\n", func, line,
                   tree->u[0].s ? tree->u[0].s : "(null)");
            break;
            
        case nPrim:
            fprintf(stderr, "[%s:%d]   primitive: \"%s\"\n", func, line,
                   tree->u[0].s ? tree->u[0].s : "(null)");
            break;
            
        case nCall:
            fprintf(stderr, "[%s:%d]   call:\n", func, line);
            if (tree->u[0].p) {
                debug_print_tree(tree->u[0].p, "    function", func, line);
            }
            if (tree->u[1].p) {
                debug_print_tree(tree->u[1].p, "    args", func, line);
            }
            break;
            
        case nThunk:
            fprintf(stderr, "[%s:%d]   thunk:\n", func, line);
            if (tree->u[0].p) {
                debug_print_tree(tree->u[0].p, "    body", func, line);
            }
            break;
            
        case nList:
            fprintf(stderr, "[%s:%d]   list:\n", func, line);
            if (tree->u[0].p) {
                debug_print_tree(tree->u[0].p, "    car", func, line);
            }
            if (tree->u[1].p) {
                debug_print_tree(tree->u[1].p, "    cdr", func, line);
            }
            break;
            
        default:
            fprintf(stderr, "[%s:%d]   (unknown tree kind %d)\n", func, line, tree->kind);
            break;
    }
    
    fflush(stderr);
}

#endif /* ES_DEBUG */