/* vec.c -- argv[] and envp[] vectors ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"

DefineTag(Vector, static);

/* mkvector -- create a new vector with specified capacity
 * Arguments:
 *   capacity: maximum number of elements the vector can hold
 * Returns: newly allocated vector with count=0 and all elements set to NULL
 */
extern Vector *mkvector(int capacity)
{   int vector_index;
    Vector *new_vector = gcalloc(offsetof(Vector, vector[capacity + 1]), &VectorTag);
    
    new_vector->alloclen = capacity;
    new_vector->count    = 0;
    
    /* Initialize all slots to NULL (including sentinel) */
    for (vector_index = 0; vector_index <= capacity; vector_index++)
        new_vector->vector[vector_index] = NULL;
        
    return new_vector;
}

static void *VectorCopy(void *original_vector)
{   size_t  allocation_size = offsetof(Vector, vector[((Vector *)original_vector)->alloclen + 1]);
    void   *new_vector      = gcalloc(allocation_size, &VectorTag);
    
    memcpy(new_vector, original_vector, allocation_size);
    return new_vector;
}

static size_t VectorScan(void *vector_ptr)
{   Vector *vector = vector_ptr;
    int element_index;
    int element_count = vector->count;
    
    /* Forward all pointers including the NULL sentinel */
    for (element_index = 0; element_index <= element_count; element_index++)
        vector->vector[element_index] = forward(vector->vector[element_index]);
        
    return offsetof(Vector, vector[vector->alloclen + 1]);
}

/* vectorize -- convert a list to a vector of strings
 * Arguments:
 *   list: linked list to convert
 * Returns: vector containing string representations of all list elements
 *          The vector will have a NULL sentinel at the end for use with execv() etc.
 */
extern Vector *vectorize(List *list)
{   int element_index;
    int list_length = length(list);

    Ref(Vector *, result_vector, NULL);
    Ref(List   *, current_list,  list);
    
    result_vector        = mkvector(list_length);
    result_vector->count = list_length;

    /* Copy string values from list to vector */
    for (element_index = 0; current_list != NULL; current_list = current_list->next, element_index++)
    {   char *string_value                   = getstr(current_list->term); /* must evaluate before assignment */
        result_vector->vector[element_index] = string_value;
    }

    RefEnd(current_list);
    RefReturn(result_vector);
}

/* qstrcmp -- string comparison wrapper for qsort()
 * Arguments:
 *   string1, string2: pointers to char* pointers (as required by qsort)
 * Returns: negative, zero, or positive value for string comparison
 */
extern int qstrcmp(const void *string1, const void *string2)
{   return strcmp(*(const char **)string1, *(const char **)string2);
}

/* sortvector -- sort vector elements alphabetically in place
 * Arguments:
 *   vector: vector to sort (must have NULL sentinel at vector[count])
 */
extern void sortvector(Vector *vector)
{   assert(vector->vector[vector->count] == NULL);
    qsort(vector->vector, vector->count, sizeof(char *), qstrcmp);
}
