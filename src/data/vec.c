/* vec.c -- argv[] and envp[] vectors ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"

DefineTag(Vector, static);

/* Forward declaration to allow use before definition */
extern Vector *vectorresize(Vector *vector, int new_capacity);

/* mkvector -- create a new vector with specified capacity
 * Arguments:
 *   capacity: maximum number of elements the vector can hold
 * Returns: newly allocated vector with count=0 and all elements set to NULL
 */
extern Vector *mkvector(int capacity)
{   int     vector_index;
    Vector *new_vector = gcalloc(offsetof(Vector, vector[capacity + 1]), &VectorTag);
    
    new_vector->alloclen = capacity;
    new_vector->count    = 0;
    
    /* Initialize all slots to NULL (including sentinel) */
    for (vector_index = 0; vector_index <= capacity; vector_index++)
        new_vector->vector[vector_index] = NULL;
        
    return new_vector;
}

/* vectorappend -- add element to end of vector, resizing if necessary
 * Arguments:
 *   vector: vector to append to
 *   string: string to add (will be copied)
 * Returns: TRUE if successful, FALSE if resize failed
 */
extern Boolean vectorappend(Vector *vector, const char *string)
{   char *string_copy;
    
    assert(vector != NULL);
    assert(string != NULL);
    
    /* Resize if necessary */
    if (vector->count >= vector->alloclen)
    {   int     new_capacity = vector->alloclen * 2;
        Vector *resized_vector;
        
        if (new_capacity < 4)
            new_capacity = 4;
            
        resized_vector = vectorresize(vector, new_capacity);
        if (resized_vector == NULL)
            return FALSE;
    }
    
    string_copy                       = gcdup(string);
    vector->vector[vector->count]     = string_copy;
    vector->vector[vector->count + 1] = NULL;  /* maintain sentinel */
    vector->count++;
    
    return TRUE;
}

/* vectorresize -- change vector capacity
 * Arguments:
 *   vector: vector to resize
 *   new_capacity: new maximum capacity
 * Returns: resized vector, or NULL if allocation failed
 */
extern Vector *vectorresize(Vector *vector, int new_capacity)
{   Vector *new_vector;
    int     copy_count;
    int     element_index;
    
    assert(vector != NULL);
    assert(new_capacity >= vector->count);
    
    new_vector = mkvector(new_capacity);
 
    if (new_vector == NULL)
        return NULL;
        
    copy_count = (vector->count < new_capacity) ? vector->count : new_capacity;
    
    /* Copy existing elements */
    for (element_index = 0; element_index < copy_count; element_index++)
        new_vector->vector[element_index] = vector->vector[element_index];
        
    new_vector->count = copy_count;
    return new_vector;
}

/* vectorget -- get element at index with bounds checking
 * Arguments:
 *   vector: vector to access
 *   index: zero-based index
 * Returns: string at index, or NULL if index out of bounds
 */
extern char *vectorget(Vector *vector, int index)
{   assert(vector != NULL);
    
    if (index < 0 || index >= vector->count)
        return NULL;
        
    return vector->vector[index];
}

/* vectorset -- set element at index with bounds checking
 * Arguments:
 *   vector: vector to modify
 *   index: zero-based index  
 *   string: new string value
 * Returns: TRUE if successful, FALSE if index out of bounds
 */
extern Boolean vectorset(Vector *vector, int index, const char *string)
{   char *string_copy;
    
    assert(vector != NULL);
    assert(string != NULL);
    
    if (index < 0 || index >= vector->count)
        return FALSE;
        
    string_copy              = gcdup(string);
    vector->vector[index]    = string_copy;
    
    return TRUE;
}

/* vectorcopy -- make a complete copy of a vector
 * Arguments:
 *   source_vector: vector to copy
 * Returns: new vector with same contents, or NULL if allocation failed
 */
extern Vector *vectorcopy(Vector *source_vector)
{   Vector *new_vector;
    int     element_index;
    
    assert(source_vector != NULL);
    
    new_vector = mkvector(source_vector->alloclen);
    if (new_vector == NULL)
        return NULL;
        
    new_vector->count = source_vector->count;
    
    /* Copy all elements */
    for (element_index = 0; element_index < source_vector->count; element_index++)
        new_vector->vector[element_index] = gcdup(source_vector->vector[element_index]);
        
    /* Ensure NULL sentinel */
    new_vector->vector[new_vector->count] = NULL;
    
    return new_vector;
}

/* vectorisempty -- check if vector has no elements
 * Arguments:
 *   vector: vector to check
 * Returns: TRUE if empty, FALSE otherwise
 */
extern Boolean vectorisempty(Vector *vector)
{   assert(vector != NULL);
    return vector->count == 0;
}

/* vectorcapacity -- get current capacity of vector
 * Arguments:
 *   vector: vector to query
 * Returns: maximum number of elements vector can hold
 */
extern int vectorcapacity(Vector *vector)
{   assert(vector != NULL);
    return vector->alloclen;
}

/* vectorfind -- find index of string in vector
 * Arguments:
 *   vector: vector to search
 *   string: string to find
 * Returns: index of first match, or -1 if not found
 */
extern int vectorfind(Vector *vector, const char *string)
{   int element_index;
    
    assert(vector != NULL);
    assert(string != NULL);
    
    for (element_index = 0; element_index < vector->count; element_index++)
    {   if (vector->vector[element_index] != NULL && 
            strcmp(vector->vector[element_index], string) == 0)
            return element_index;
    }
    
    return -1;
}

/* vectorcontains -- check if vector contains string
 * Arguments:
 *   vector: vector to search
 *   string: string to find
 * Returns: TRUE if found, FALSE otherwise
 */
extern Boolean vectorcontains(Vector *vector, const char *string)
{   return vectorfind(vector, string) != -1;
}

/* vectorconcat -- combine two vectors into a new vector
 * Arguments:
 *   first_vector: first vector
 *   second_vector: second vector
 * Returns: new vector containing all elements, or NULL if allocation failed
 */
extern Vector *vectorconcat(Vector *first_vector, Vector *second_vector)
{   Vector *result_vector;
    int     total_capacity;
    int     element_index;
    int     result_index  = 0;
    
    assert(first_vector  != NULL);
    assert(second_vector != NULL);
    
    total_capacity = first_vector->count + second_vector->count;
    result_vector  = mkvector(total_capacity);
    
    if (result_vector == NULL)
        return NULL;
        
    /* Copy elements from first vector */
    for (element_index = 0; element_index < first_vector->count; element_index++)
        result_vector->vector[result_index++] = gcdup(first_vector->vector[element_index]);
        
    /* Copy elements from second vector */
    for (element_index = 0; element_index < second_vector->count; element_index++)
        result_vector->vector[result_index++] = gcdup(second_vector->vector[element_index]);
        
    result_vector->count                 = total_capacity;
    result_vector->vector[result_index] = NULL;  /* sentinel */
    
    return result_vector;
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
