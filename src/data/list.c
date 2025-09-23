/* list.c -- operations on lists ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"

/*
 * Allocation and garbage collector support
 */

DefineTag(List, static);

/* mklist -- create a new list node with given term and next pointer */
extern List *mklist(Term *term, List *next)
{   gcdisable();
    assert(term != NULL);
    Ref(List *, list, gcnew(List));
    list->term = term;
    list->next = next;
    gcenable();
    RefReturn(list);
}

static void *ListCopy(void *original_ptr)
{   void *new_ptr = gcnew(List);
    memcpy(new_ptr, original_ptr, sizeof(List));
    return new_ptr;
}

static size_t ListScan(void *list_ptr)
{   List *list = list_ptr;
    list->term = forward(list->term);
    list->next = forward(list->next);
    return sizeof(List);
}

/*
 * Basic list manipulations
 */

/* reverse -- destructively reverse a list in O(n) time */
extern List *reverse(List *list)
{   List *previous_node;
    List *next_node;
    
    if (list == NULL)
        return NULL;
        
    previous_node = NULL;
    do
    {   next_node     = list->next;
        list->next    = previous_node;
        previous_node = list;
    }
	while ((list = next_node) != NULL);
    
    return previous_node;
}

/* append -- merge two lists non-destructively with smart memory reservation */
extern List *append(List *head, List *tail)
{   List *current_list;
    List **previous_ptr;
    
    Ref(List *, head_ptr, head);
    Ref(List *, tail_ptr, tail);
    
    /* Pre-allocate memory to reduce GC pressure during construction */
    gcreserve(40 * sizeof(List));
    gcdisable();
    
    head = head_ptr;
    tail = tail_ptr;
    RefEnd2(tail_ptr, head_ptr);

    /* Copy head list nodes */
    for (previous_ptr  = &current_list; head != NULL; head = head->next)
    {   List *new_node = mklist(head->term, NULL);
        *previous_ptr  = new_node;
        previous_ptr   = &new_node->next;
    }
    
    /* Attach tail without copying */
    *previous_ptr = tail;

    Ref(List *, result, current_list);
    gcenable();
    RefReturn(result);
}

/* prepend -- add a single term to the front of a list non-destructively */
extern List *prepend(Term *term, List *list)
{   assert(term != NULL);
    return mklist(term, list);
}

/* listcopy -- make a complete copy of a list */
extern List *listcopy(List *list)
{   return append(list, NULL);
}

/* length -- count the number of elements in a list (O(n) operation) */
extern int length(List *list)
{   int element_count = 0;
    
    for (; list != NULL; list = list->next)
        ++element_count;
        
    return element_count;
}

/* listify -- convert an argc/argv array into a list (in reverse order) */
extern List *listify(int argc, char **argv)
{   Ref(List *, list, NULL);
    
    /* Build list in reverse order for efficiency */
    while (argc > 0)
    {   Term *term = mkstr(argv[--argc]);
        list       = mklist(term, list);
    }
    
    RefReturn(list);
}

/* nth -- return nth element of a list with Python-style indexing
 *        Positive indices: 1-based (1st, 2nd, 3rd, ...)
 *        Negative indices: count from end (-1 = last, -2 = second-to-last)
 *        Returns NULL if index is out of bounds
 */
extern Term *nth(List *list, int index)
{   int list_length;
    int actual_index;
    
    if (list == NULL || index == 0)
        return NULL;
    
    /* Handle negative indexing like Python */
    if (index < 0)
    {   list_length  = length(list);
        actual_index = list_length + index + 1;  /* Convert to 1-based positive */
        
        if (actual_index <= 0)
            return NULL;  /* Index too negative */
            
        index = actual_index;
    }
    
    /* Traverse to the desired position */
    for (; index > 0 && list != NULL; list = list->next)
    {   assert(list->term != NULL);
        if (--index == 0)
            return list->term;
    }
    
    return NULL;  /* Index out of bounds */
}

/* sortlist -- sort a list by converting to vector, sorting, then back to list */
extern List *sortlist(List *list)
{   if (length(list) > 1)
    {   Vector *vector_representation;
        
        vector_representation = vectorize(list);
        sortvector(vector_representation);
        
        gcdisable();
        Ref(List *, sorted_list, listify(vector_representation->count, vector_representation->vector));
        gcenable();
        
        list = sorted_list;
        RefEnd(sorted_list);
    }
    
    return list;
}
