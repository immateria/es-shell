/* str.c -- es string operations ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"
#include "print.h"

/* String allocation constants */
#define PRINT_ALLOCSIZE     64
#define BUFFER_GROWTH_FACTOR 2
#define ALIGNMENT_MASK      (PRINT_ALLOCSIZE - 1)

/* str_grow -- buffer grow function for str() and strv()
 * Called automatically when buffer needs more space during formatted printing
 */
static int str_grow(Format *format_state, size_t additional_bytes)
{   Buffer *expanded_buffer = expandbuffer(format_state->u.p, additional_bytes);
    
    format_state->u.p       = expanded_buffer;
    format_state->buf       = expanded_buffer->str + (format_state->buf - format_state->bufbegin);
    format_state->bufbegin  = expanded_buffer->str;
    format_state->bufend    = expanded_buffer->str + expanded_buffer->len;
    
    return 0;
}

/* sstrv -- internal function to print formatted string into specified buffer type
 * Arguments:
 *   seal: buffer sealing function (sealbuffer for GC, psealbuffer for pspace)
 *   format_string: printf-style format string
 *   argument_list: va_list of arguments
 * Returns: formatted string in the specified memory space
 */
static char *sstrv(char *(*seal)(Buffer *), const char *format_string, va_list argument_list)
{   Buffer *string_buffer;
    Format format_state;

    gcdisable();
    string_buffer = openbuffer(0);
    
    format_state.u.p       = string_buffer;
#if NO_VA_LIST_ASSIGN
    memcpy(format_state.args, argument_list, sizeof(va_list));
#else
    format_state.args      = argument_list;
#endif
    format_state.buf       = string_buffer->str;
    format_state.bufbegin  = string_buffer->str;
    format_state.bufend    = string_buffer->str + string_buffer->len;
    format_state.grow      = str_grow;
    format_state.flushed   = 0;

    printfmt(&format_state, format_string);
    fmtputc(&format_state, '\0');
    gcenable();

    return seal(format_state.u.p);
}

/* strv -- create formatted string in GC space using va_list
 * Arguments:
 *   format_string: printf-style format string  
 *   argument_list: va_list of arguments
 * Returns: formatted string in garbage collection space (temporary)
 */
extern char *strv(const char *format_string, va_list argument_list)
{   return sstrv(sealbuffer, format_string, argument_list);
}

/* str -- create formatted string in GC space using variadic arguments
 * Arguments:
 *   format_string: printf-style format string
 *   ...: arguments for format string
 * Returns: formatted string in garbage collection space (will be cleaned up automatically)
 * Usage: char *msg = str("Hello %s, you have %d messages", name, count);
 */
extern char *str VARARGS1(const char *, format_string)
{   char    *result_string;
    va_list  argument_list;
    
    VA_START(argument_list, format_string);
    result_string = strv(format_string, argument_list);
    va_end(argument_list);
    
    return result_string;
}

/* pstr -- create formatted string in persistent space using variadic arguments
 * Arguments:
 *   format_string: printf-style format string
 *   ...: arguments for format string  
 * Returns: formatted string in persistent space (survives garbage collection)
 * Usage: char *permanent = pstr("Config: %s=%s", key, value);
 */
extern char *pstr VARARGS1(const char *, format_string)
{   char    *result_string;
    va_list  argument_list;
    
    VA_START(argument_list, format_string);
    result_string = sstrv(psealbuffer, format_string, argument_list);
    va_end(argument_list);
    
    return result_string;
}

/* mprint_grow -- buffer grow function for mprint()
 * Uses malloc/realloc for memory management instead of GC
 */
static int mprint_grow(Format *format_state, size_t additional_bytes)
{   char   *new_buffer;
    size_t  current_length = format_state->bufend - format_state->bufbegin + 1;
    size_t  required_length;
    
    /* Calculate new buffer size with alignment */
    required_length = (current_length >= additional_bytes)
        ? current_length * BUFFER_GROWTH_FACTOR
        : ((current_length + additional_bytes) + PRINT_ALLOCSIZE) &~ ALIGNMENT_MASK;
        
    new_buffer = erealloc(format_state->bufbegin, required_length);
    
    format_state->buf       = new_buffer + (format_state->buf - format_state->bufbegin);
    format_state->bufbegin  = new_buffer;
    format_state->bufend    = new_buffer + required_length - 1;
    
    return 0;
}

/* mprint -- create formatted string in malloc space using variadic arguments
 * Arguments:
 *   format_string: printf-style format string
 *   ...: arguments for format string
 * Returns: formatted string allocated with malloc (must be freed by caller)
 * Usage: char *external = mprint("Error: %s at line %d", error, line);
 *        // Remember to free(external) when done
 */
extern char *mprint VARARGS1(const char *, format_string)
{   Format format_state;
    
    format_state.u.n = 1;
    VA_START(format_state.args, format_string);

    format_state.buf       = ealloc(PRINT_ALLOCSIZE);
    format_state.bufbegin  = format_state.buf;
    format_state.bufend    = format_state.buf + PRINT_ALLOCSIZE - 1;
    format_state.grow      = mprint_grow;
    format_state.flushed   = 0;

    printfmt(&format_state, format_string);
    *format_state.buf = '\0';
    va_end(format_state.args);
    
    return format_state.bufbegin;
}

/*
 * StrList -- lists of strings for shell operations
 * Used for quote lists, option processing, and other string collections
 * Note: This is probably a premature optimization but kept for compatibility
 */

DefineTag(StrList, static);

/* mkstrlist -- create a new string list node
 * Arguments:
 *   string: string to store (must not be NULL)
 *   next: next node in list, or NULL for end
 * Returns: new StrList node in GC space
 */
extern StrList *mkstrlist(char *string, StrList *next)
{   gcdisable();
    assert(string != NULL);
    
    Ref(StrList *, new_list, gcnew(StrList));
    new_list->str  = string;
    new_list->next = next;
    gcenable();
    
    RefReturn(new_list);
}

static void *StrListCopy(void *original_ptr)
{   void *new_ptr = gcnew(StrList);
    memcpy(new_ptr, original_ptr, sizeof(StrList));
    return new_ptr;
}

static size_t StrListScan(void *strlist_ptr)
{   StrList *string_list = strlist_ptr;
    
    string_list->str  = forward(string_list->str);
    string_list->next = forward(string_list->next);
    
    return sizeof(StrList);
}
