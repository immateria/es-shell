/* glom.c -- walk parse tree to produce list ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"

/* concat -- cartesian cross product concatenation
 * Combines two lists by concatenating every element from list1 with every element from list2
 * Example: list1=[a,b], list2=[1,2] -> result=[a1,a2,b1,b2]
 */
extern List *concat(List *first_list, List *second_list)
{   List **list_ptr;
    List  *result   = NULL;

    gcdisable();
    for (list_ptr = &result; first_list != NULL; first_list = first_list->next)
    {   List *current_item;
        for (current_item   =  second_list; current_item != NULL; current_item = current_item->next)
        {    List *new_node =  mklist(termcat(first_list->term, current_item->term), NULL);
            *list_ptr       =  new_node;
             list_ptr       = &new_node->next;
        }
    }

    Ref(List *, list, result);
    gcenable();
    RefReturn(list);
}

/* qcat -- concatenate two quote flag terms */
static char *qcat(const char *quote1, const char *quote2, Term *term1, Term *term2)
{   size_t  length1;
    size_t  length2;
    char   *result;
    char   *string_ptr;

    assert(gcisblocked());

    if (quote1 == QUOTED && quote2 == QUOTED)
        return QUOTED;
 
    if (quote1 == UNQUOTED && quote2 == UNQUOTED)
        return UNQUOTED;

    length1 = (quote1 == QUOTED || quote1 == UNQUOTED) ? strlen(getstr(term1)) : strlen(quote1);
    length2 = (quote2 == QUOTED || quote2 == UNQUOTED) ? strlen(getstr(term2)) : strlen(quote2);
    result  = string_ptr = gcalloc(length1 + length2 + 1, &StringTag);

    if (quote1 == QUOTED)
        memset(string_ptr, 'q', length1);
		
    else if (quote1 == UNQUOTED)
        memset(string_ptr, 'r', length1);
		
    else
        memcpy(string_ptr, quote1, length1);
 
    string_ptr += length1;
    
    if (quote2 == QUOTED)
        memset(string_ptr, 'q', length2);
		
    else if (quote2 == UNQUOTED)
        memset(string_ptr, 'r', length2);
		
    else
        memcpy(string_ptr, quote2, length2);
 
    string_ptr  += length2;
    *string_ptr  = '\0';

    return result;
}

/* qconcat -- cartesian cross product concatenation; also produces a quote list */
static List *qconcat(List *first_list, List *second_list, StrList *quote_list1, StrList *quote_list2, StrList **quote_result)
{   List    **list_ptr;
    List     *result    = NULL;
    StrList **quote_ptr;

    gcdisable();
    for (list_ptr = &result, quote_ptr = quote_result; first_list != NULL; first_list = first_list->next, quote_list1 = quote_list1->next)
    {   List    *current_item;
        StrList *current_quote;
        
        for (current_item = second_list, current_quote = quote_list2; current_item != NULL; current_item = current_item->next, current_quote = current_quote->next)
        {   List    *new_node;
            StrList *new_quote;
            
             new_node  =  mklist(termcat(first_list->term, current_item->term), NULL);
            *list_ptr  =  new_node;
             list_ptr  = &new_node->next;
            
             new_quote =  mkstrlist(qcat(quote_list1->str, current_quote->str, first_list->term, current_item->term), NULL);
            *quote_ptr =  new_quote;
             quote_ptr = &new_quote->next;
        }
    }

    Ref(List *, list, result);
    gcenable();
    RefReturn(list);
}

/* subscript -- variable subscripting with range support
 * Supports syntax like: $var(1), $var(2...5), $var(1...)
 */
static List *subscript(List *list, List *subscript_list)
{   int    low_index;
    int    high_index;
    int    list_length;
    int    current_position;

    List  *result;
    List **previous_ptr;
    List  *current_node;

    gcdisable();

    result           =  NULL;
    previous_ptr     = &result;
    list_length      =  length(list);
    current_node     =  list;
    current_position =  1;

    if (subscript_list != NULL && streq(getstr(subscript_list->term), "..."))
    {   low_index = 1;
        goto mid_range;
    }

    while (subscript_list != NULL)
    {   low_index = atoi(getstr(subscript_list->term));
        if (low_index < 1)
        {   Ref(char *, bad_subscript, getstr(subscript_list->term));
            gcenable();
            fail("es:subscript", "bad subscript: %s", bad_subscript);
            RefEnd(bad_subscript);
        }
        subscript_list = subscript_list->next;
        
        if (subscript_list != NULL && streq(getstr(subscript_list->term), "..."))
        {
        mid_range:
            subscript_list = subscript_list->next;
			
            if (subscript_list == NULL)
                high_index = list_length;
				
            else
            {   high_index = atoi(getstr(subscript_list->term));
                if (high_index < 1)
                {   Ref(char *, bad_subscript, getstr(subscript_list->term));
                    gcenable();
	
                    fail("es:subscript", "bad subscript: %s", bad_subscript);
                    RefEnd(bad_subscript);
                }
				 
                if (high_index > list_length)
                    high_index = list_length;
				 
                subscript_list = subscript_list->next;
            }
        }
			
        else
            high_index = low_index;
            
        if (low_index > list_length)
            continue;
            
        if (current_position > low_index)
        {   current_node     = list;
            current_position = 1;
        }
        
        for (; current_position < low_index; current_position++, current_node = current_node->next)
            ;
            
        for (; current_position <= high_index; current_position++, current_node = current_node->next)
        {   *previous_ptr = mklist(current_node->term, NULL);
            previous_ptr  = &(*previous_ptr)->next;
        }
    }

    Ref(List *, result_ref, result);
    gcenable();
    RefReturn(result_ref);
}

/* glom1 -- glom when we don't need to produce a quote list */
static List *glom1(Tree *tree, Binding *binding)
{   Ref(List    *, result,      NULL);
    Ref(List    *, tail,        NULL);
    Ref(Tree    *, tree_ptr,    tree);
    Ref(Binding *, binding_ptr, binding);

    assert(!gcisblocked());

    while (tree_ptr != NULL)
    {   Ref(List *, list, NULL);

        switch (tree_ptr->kind)
        {
        case nQword:
            list     = mklist(mkterm(tree_ptr->u[0].s, NULL), NULL);
            tree_ptr = NULL;
            break;
            
        case nWord:
            list     = mklist(mkterm(tree_ptr->u[0].s, NULL), NULL);
            tree_ptr = NULL;
            break;
            
        case nThunk:
        case nLambda:
            list     = mklist(mkterm(NULL, mkclosure(tree_ptr, binding_ptr)), NULL);
            tree_ptr = NULL;
            break;
            
        case nPrim:
            list     = mklist(mkterm(NULL, mkclosure(tree_ptr, NULL)), NULL);
            tree_ptr = NULL;
            break;
            
        case nVar:
            Ref(List *, variable_name, glom1(tree_ptr->u[0].p, binding_ptr));
            tree_ptr = NULL;
            
            for (; variable_name != NULL; variable_name = variable_name->next)
            {   list = listcopy(varlookup(getstr(variable_name->term), binding_ptr));
                if (list != NULL)
                {   if (result == NULL)
                        tail = result = list;
					
                    else
                        tail->next = list;
				 
                    for (; tail->next != NULL; tail = tail->next)
                        ;
                }
                list = NULL;
            }
            RefEnd(variable_name);
            break;
            
        case nVarsub:
            list = glom1(tree_ptr->u[0].p, binding_ptr);
			
            if (list == NULL)
                fail("es:glom", "null variable name in subscript");
			
            if (list->next != NULL)
                fail("es:glom", "multi-word variable name in subscript");
                
            Ref(char *, variable_name, getstr(list->term));
            list     = varlookup(variable_name, binding_ptr);
			
            Ref(List *, subscript_expr, glom1(tree_ptr->u[1].p, binding_ptr));
            tree_ptr = NULL;
            list     = subscript(list, subscript_expr);
			
            RefEnd2(subscript_expr, variable_name);
            break;
            
        case nCall:
            list     = listcopy(walk(tree_ptr->u[0].p, binding_ptr, 0));
            tree_ptr = NULL;
            break;
            
        case nList:
            list     = glom1(tree_ptr->u[0].p, binding_ptr);
            tree_ptr = tree_ptr->u[1].p;
            break;
            
        case nConcat:
            Ref(List *, left_list,  glom1(tree_ptr->u[0].p, binding_ptr));
            Ref(List *, right_list, glom1(tree_ptr->u[1].p, binding_ptr));
            tree_ptr = NULL;
            list     = concat(left_list, right_list);
			
            RefEnd2(right_list, left_list);
            break;
            
        default:
            fail("es:glom", "glom1: bad node kind %d", tree->kind);
        }

        if (list != NULL)
        {   if (result == NULL)
                tail = result = list;
			
            else
                tail->next = list;
		 
            for (; tail->next != NULL; tail = tail->next)
                ;
        }
        RefEnd(list);
    }

    RefEnd3(binding_ptr, tree_ptr, tail);
    RefReturn(result);
}

/* glom2 -- glom and produce a quoting list */
extern List *glom2(Tree *tree, Binding *binding, StrList **quote_result)
{   Ref(List    *, result,      NULL);
    Ref(List    *, tail,        NULL);
    Ref(StrList *, quote_tail,  NULL);
    Ref(Tree    *, tree_ptr,    tree);
    Ref(Binding *, binding_ptr, binding);

    assert(!gcisblocked());
    assert(quote_result != NULL);

    /*
     * This loop covers only the cases where we might produce some
     * unquoted (raw) values. All other cases are handled in glom1
     * and we just add quoted word flags to them.
     */

    while (tree_ptr != NULL)
    {   Ref(List    *, list,       NULL);
        Ref(StrList *, quote_list, NULL);

        switch (tree_ptr->kind)
        {
        case nWord:
            list       = mklist(mkterm(tree_ptr->u[0].s, NULL), NULL);
            quote_list = mkstrlist(UNQUOTED, NULL);
            tree_ptr   = NULL;
            break;
            
        case nList:
            list     = glom2(tree_ptr->u[0].p, binding_ptr, &quote_list);
            tree_ptr = tree_ptr->u[1].p;
            break;
            
        case nConcat:
            Ref(List    *, left_list,   NULL);
            Ref(List    *, right_list,  NULL);
            Ref(StrList *, left_quote,  NULL);
            Ref(StrList *, right_quote, NULL);
            
            left_list  = glom2(tree_ptr->u[0].p, binding_ptr, &left_quote);
            right_list = glom2(tree_ptr->u[1].p, binding_ptr, &right_quote);
            list       = qconcat(left_list, right_list, left_quote, right_quote, &quote_list);
			
            RefEnd4(right_quote, left_quote, right_list, left_list);
            tree_ptr   = NULL;
            break;
            
        default:
            list = glom1(tree_ptr, binding_ptr);
            Ref(List *, list_item, list);
			
            for (; list_item != NULL; list_item = list_item->next)
                quote_list = mkstrlist(QUOTED, quote_list);
			
            RefEnd(list_item);
            tree_ptr = NULL;
            break;
        }

        if (list != NULL)
        {   if (result == NULL)
            {   assert(*quote_result == NULL);
                 result       = tail = list;
                *quote_result = quote_tail = quote_list;
            }
			
            else
            {   assert(*quote_result != NULL);
                tail->next       = list;
                quote_tail->next = quote_list;
            }
            
            for (; tail->next != NULL; tail = tail->next, quote_tail = quote_tail->next)
                ;
		 
            assert(quote_tail->next == NULL);
        }
        RefEnd2(quote_list, list);
    }

    RefEnd4(binding_ptr, tree_ptr, quote_tail, tail);
    RefReturn(result);
}

/* glom -- top level glom dispatching
 * If globit is true, performs globbing on the result
 */
extern List *glom(Tree *tree, Binding *binding, Boolean globit)
{   if (globit)
    {   Ref(List    *, list,  NULL);
        Ref(StrList *, quote, NULL);
        
        list = glom2(tree, binding, &quote);
        list = glob(list, quote);
		
        RefEnd(quote);
        RefReturn(list);
    }
	 
    else
        return glom1(tree, binding);
}
