/* dict.c -- hash-table based dictionaries ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"

#define INIT_DICT_SIZE  2
#define REMAIN(n)       (((n) * 2) / 3)
#define GROW(n)          ((n) * 2)

/* FNV-1a hash constants */
#define FNV_OFFSET      14695981039346656037ULL
#define FNV_PRIME       1099511628211ULL

/* Special pointer value for deleted entries */
#define DELETED_ENTRY   ((char*)1)

/*
 * FNV-1a hash functions
 */

static unsigned long strhash2(const char *first_string, const char *second_string)
{   unsigned long hash_value = FNV_OFFSET;
    const unsigned char *current_char;
    
    /* Hash first string */
    current_char = (const unsigned char *)first_string;
    while (*current_char)
    {   hash_value ^= *current_char++;
        hash_value *= FNV_PRIME;
    }
    
    /* Hash second string if present */
    if (second_string)
    {   current_char = (const unsigned char *)second_string;
        while (*current_char)
        {   hash_value ^= *current_char++;
            hash_value *= FNV_PRIME;
        }
    }
    
    return hash_value;
}

static unsigned long strhash(const char *string)
{   return strhash2(string, NULL);
}

/*
 * Helper functions for entry state checking
 */

static inline Boolean is_deleted_entry(const char *entry_name)
{   return entry_name == DELETED_ENTRY;
}

static inline Boolean is_empty_entry(const char *entry_name)
{   return entry_name == NULL;
}

static inline Boolean is_valid_entry(const char *entry_name)
{   return entry_name != NULL && entry_name != DELETED_ENTRY;
}

/*
 * Data structures and garbage collection
 */

DefineTag(Dict, static);

typedef struct
{   char *name;
    void *value;
} Assoc;

struct Dict
{   int   size;
    int   remain;
    Assoc table[1];  /* variable length */
};

static Dict *mkdict0(int initial_size)
{   size_t  allocation_length = offsetof(Dict, table[initial_size]);
    Dict   *new_dict          = gcalloc(allocation_length, &DictTag);
    
    memzero(new_dict, allocation_length);
    new_dict->size   = initial_size;
    new_dict->remain = REMAIN(initial_size);
 
    return new_dict;
}

static void *DictCopy(void *original_ptr)
{   Dict   *original_dict     = original_ptr;
    size_t  allocation_length = offsetof(Dict, table[original_dict->size]);
    void   *new_ptr           = gcalloc(allocation_length, &DictTag);
    
    memcpy(new_ptr, original_ptr, allocation_length);
    return new_ptr;
}

static size_t DictScan(void *dict_ptr)
{   Dict *current_dict = dict_ptr;
    int table_index;
    
    for (table_index = 0; table_index < current_dict->size; table_index++)
    {   Assoc *current_assoc = &current_dict->table[table_index];
        current_assoc->name  = forward(current_assoc->name);
        current_assoc->value = forward(current_assoc->value);
    }
    return offsetof(Dict, table[current_dict->size]);
}

/*
 * Private operations
 */

static Assoc *get(Dict *dictionary, const char *lookup_name)
{   unsigned  long hash_value = strhash(lookup_name);
    unsigned  long table_mask = dictionary->size - 1;
    Assoc    *current_entry;
    
    for (; (current_entry = &dictionary->table[hash_value & table_mask])->name != NULL; hash_value++)
    {   if (!is_deleted_entry(current_entry->name) && streq(lookup_name, current_entry->name))
            return current_entry;
    }
    return NULL;
}

static void recurseput(void *, char *, void *);

static Dict *put(Dict *dictionary, char *entry_name, void *entry_value)
{   unsigned  long hash_value;
    unsigned  long table_mask;
    Assoc    *target_entry;
    
    assert(get(dictionary, entry_name) == NULL);
    assert(entry_value                 != NULL);

    if (dictionary->remain <= 1)
    {   Dict *new_dictionary;
        Ref(Dict *, old_dictionary, dictionary);
        Ref(char *, name_ptr,       entry_name);
        Ref(void *, value_ptr,      entry_value);
        
        new_dictionary = mkdict0(GROW(old_dictionary->size));
        dictforall(old_dictionary, recurseput, new_dictionary);
        dictionary  = new_dictionary;
        entry_name  = name_ptr;
        entry_value = value_ptr;
        RefEnd3(value_ptr, name_ptr, old_dictionary);
    }

    hash_value = strhash(entry_name);
    table_mask = dictionary->size - 1;
    
    for (; (target_entry = &dictionary->table[hash_value & table_mask])->name != DELETED_ENTRY; hash_value++)
    {   if (target_entry->name == NULL)
        {   --dictionary->remain;
            break;
        }
    }

    target_entry->name  = entry_name;
    target_entry->value = entry_value;
			
    return dictionary;
}

static void recurseput(void *dict_ptr, char *name_str, void *value_ptr)
{   put(dict_ptr, name_str, value_ptr);
}

static void rm(Dict *dictionary, Assoc *target_entry)
{   unsigned  long probe_position;
    unsigned  long table_mask;
    Assoc    *probe_entry;
    
    assert(dictionary->table <= target_entry && target_entry < &dictionary->table[dictionary->size]);

    target_entry->name  = DELETED_ENTRY;
    target_entry->value = NULL;
    probe_position      = target_entry - dictionary->table;
    table_mask          = dictionary->size - 1;
    
    for (probe_position++; (probe_entry = &dictionary->table[probe_position & table_mask])->name == DELETED_ENTRY; probe_position++)
        ;
 
    if (probe_entry->name != NULL)
        return;
        
    for (probe_position--; (probe_entry = &dictionary->table[probe_position & table_mask])->name == DELETED_ENTRY; probe_position--)
    {   probe_entry->name = NULL;
        ++dictionary->remain;
    }
}

/*
 * Exported functions
 */

extern Dict *mkdict(void)
{   return mkdict0(INIT_DICT_SIZE);
}

extern void *dictget(Dict *dictionary, const char *lookup_name)
{   Assoc *found_entry = get(dictionary, lookup_name);
    
    if (found_entry == NULL)
        return NULL;
 
    return found_entry->value;
}

extern Dict *dictput(Dict *dictionary, char *entry_name, void *entry_value)
{   Assoc *existing_entry = get(dictionary, entry_name);
    
    if (entry_value != NULL)
    {   if (existing_entry == NULL)
            dictionary = put(dictionary, entry_name, entry_value);
		
        else
            existing_entry->value = entry_value;
    }

    else if (existing_entry != NULL)
        rm(dictionary, existing_entry);
        
    return dictionary;
}

extern void dictforall(Dict *target_dict, void (*processor_func)(void *, char *, void *), void *callback_arg)
{   int table_index;
    Ref(Dict *, dictionary_ref, target_dict);
    Ref(void *, argument_ref,   callback_arg);
    
    for (table_index = 0; table_index < dictionary_ref->size; table_index++)
    {   Assoc *current_entry = &dictionary_ref->table[table_index];
        
        if (is_valid_entry(current_entry->name))
            (*processor_func)(argument_ref, current_entry->name, current_entry->value);
    }
    RefEnd2(argument_ref, dictionary_ref);
}

extern void *dictget2(Dict *dictionary, const char *first_name, const char *second_name)
{   unsigned  long hash_value = strhash2(first_name, second_name);
    unsigned  long table_mask = dictionary->size - 1;
    Assoc    *current_entry;
    
    for (; (current_entry = &dictionary->table[hash_value & table_mask])->name != NULL; hash_value++)
    {   if (!is_deleted_entry(current_entry->name) && streq2(current_entry->name, first_name, second_name))
            return current_entry->value;
    }
    return NULL;
}
