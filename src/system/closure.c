/* closure.c -- operations on bindings, closures, lambdas, and thunks ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"

/*
 * Closure garbage collection support
 */

DefineTag(Closure, static);

extern Closure *mkclosure(Tree *tree, Binding *binding)
{   gcdisable();
    Ref(Closure *, closure, gcnew(Closure));
    closure->tree    = tree;
    closure->binding = binding;
    gcenable();
    RefReturn(closure);
}

static void *ClosureCopy(void *original_ptr)
{   void *new_ptr = gcnew(Closure);
    memcpy(new_ptr, original_ptr, sizeof(Closure));
    return new_ptr;
}

static size_t ClosureScan(void *closure_ptr)
{   Closure *closure = closure_ptr;
    closure->tree    = forward(closure->tree);
    closure->binding = forward(closure->binding);
    return sizeof(Closure);
}

/* revtree -- destructively reverse a list stored in a tree */
static Tree *revtree(Tree *tree)
{   Tree *previous_node;
    Tree *next_node;
    
    if (tree == NULL)
        return NULL;
        
    previous_node = NULL;
    do
    {   assert(tree->kind == nList);
        next_node     = tree->u[1].p;
        tree->u[1].p  = previous_node;
        previous_node = tree;
	 
    }
	while ((tree = next_node) != NULL);
    
    return previous_node;
}

typedef struct Chain Chain;
struct Chain
{   Closure *closure;
    Chain   *next;
};
static Chain *chain = NULL;

static Binding *extract(Tree *tree, Binding *bindings)
{   assert(gcisblocked());

    for (; tree != NULL; tree = tree->u[1].p)
    {   Tree *definition = tree->u[0].p;
        assert(tree->kind == nList);
        
        if (definition != NULL)
        {   List *value_list    = NULL;
            Tree *variable_name = definition->u[0].p;
            
            assert(variable_name->kind == nWord || variable_name->kind == nQword);
            definition = revtree(definition->u[1].p);
            
            for (; definition != NULL; definition = definition->u[1].p)
            {   Term     *term;
                Tree     *word      = definition->u[0].p;
                NodeKind  word_kind = word->kind;
                
                assert(definition->kind == nList);
                assert(word_kind == nWord || word_kind == nQword || word_kind == nPrim);
                
                if (word_kind == nPrim)
                {   char *primitive_name = word->u[0].s;
                    
                    if (streq(primitive_name, "nestedbinding"))
                    {   int    chain_index;
                        int    chain_count;
                        Chain *chain_ptr;
                        
                        if ((definition = definition->u[1].p) == NULL
                         || definition->u[0].p->kind != nWord
                         || (chain_count = (atoi(definition->u[0].p->u[0].s))) < 0)
                        {   fail("$&parse", "improper use of $&nestedbinding");
                            NOTREACHED;
                        }
                        
                        for (chain_ptr = chain, chain_index = 0;; chain_ptr = chain_ptr->next, chain_index++)
                        {   if (chain_ptr == NULL)
                            {   fail("$&parse", "bad count in $&nestedbinding: %d", chain_count);
                                NOTREACHED;
                            }
						 
                            if (chain_index == chain_count)
                                break;
                        }
                        term = mkterm(NULL, chain_ptr->closure);
                    }
						
                    else
                    {   fail("$&parse", "bad unquoted primitive in %%closure: $&%s", primitive_name);
                        NOTREACHED;
                    }
                }
					
                else
                    term = mkstr(word->u[0].s);
                    
                value_list = mklist(term, value_list);
            }
            bindings = mkbinding(variable_name->u[0].s, value_list, bindings);
        }
    }

    return bindings;
}

extern Closure *extractbindings(Tree *input_tree)
{   Chain    current_chain_entry;
    Tree    *volatile tree     = input_tree;
    Binding *volatile bindings = NULL;

    gcdisable();

    if (tree->kind == nList && tree->u[1].p == NULL)
        tree = tree->u[0].p;

    current_chain_entry.closure = mkclosure(NULL, NULL);
    current_chain_entry.next    = chain;
    chain                       = &current_chain_entry;

    ExceptionHandler

        while (tree->kind == nClosure)
        {   bindings = extract(tree->u[0].p, bindings);
            tree     = tree->u[1].p;
		 
            if (tree == NULL)
                fail("$&parse", "null body in %%closure");
		 
            if (tree->kind == nList && tree->u[1].p == NULL)
                tree = tree->u[0].p;
        }

    CatchException(caught_exception)

        chain = chain->next;
        throw(caught_exception);

    EndExceptionHandler

    chain = chain->next;

    Ref(Closure *, result, current_chain_entry.closure);
    result->tree    = tree;
    result->binding = bindings;
    gcenable();
    RefReturn(result);
}

/*
 * Binding garbage collection support
 */

DefineTag(Binding, static);

extern Binding *mkbinding(char *name, List *definition, Binding *next)
{   assert(next == NULL || next->name != NULL);
    validatevar(name);
    gcdisable();
    Ref(Binding *, binding, gcnew(Binding));
    binding->name = name;
    binding->defn = definition;
    binding->next = next;
    gcenable();
    RefReturn(binding);
}

extern Binding *reversebindings(Binding *binding)
{   if (binding == NULL)
        return NULL;
	
    else
    {   Binding *previous_binding;
        Binding *next_binding;
        
        previous_binding = NULL;
	 
        do
        {   next_binding     = binding->next;
            binding->next    = previous_binding;
            previous_binding = binding;
        }
		while ((binding = next_binding) != NULL);
        
        return previous_binding;
    }
}

static void *BindingCopy(void *original_ptr)
{   void *new_ptr = gcnew(Binding);
    memcpy(new_ptr, original_ptr, sizeof(Binding));
 
    return new_ptr;
}

static size_t BindingScan(void *binding_ptr)
{   Binding *binding = binding_ptr;
    binding->name    = forward(binding->name);
    binding->defn    = forward(binding->defn);
    binding->next    = forward(binding->next);
    return sizeof(Binding);
}
