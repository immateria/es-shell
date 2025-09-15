/* opt.c -- option parsing ($Revision: 1.1.1.1 $) */

#include "es.h"

/* Option parsing constants */
#define OPTION_PREFIX       '-'
#define END_OF_OPTIONS      "--"
#define REQUIRES_ARGUMENT   ':'
#define OPTION_ERROR        '?'
#define MISSING_ARG_ERROR   ':'

/* Global state for option parsing */
static const char    *usage_message;
static const char    *invoker_name;
static       List    *argument_list;
static       Term    *option_argument;
static       int      next_char_position;
static       Boolean  throw_on_error;

/* esoptbegin -- initialize option parsing state
 * Arguments:
 *   list: command line arguments to parse
 *   caller: name of calling function (for error messages)
 *   usagemsg: usage string to display on errors
 *   throws: if TRUE, throw exceptions on errors; if FALSE, return error codes
 */
extern void esoptbegin(List *list, const char *caller, const char *usagemsg, Boolean throws)
{   static Boolean initialized = FALSE;
    
    if (!initialized)
    {   initialized = TRUE;
        globalroot(&usage_message);
        globalroot(&invoker_name);
        globalroot(&argument_list);
        globalroot(&option_argument);
    }
    
    assert(usage_message == NULL);
    usage_message      = usagemsg;
    invoker_name       = caller;
    argument_list      = list;
    option_argument    = NULL;
    next_char_position = 0;
    throw_on_error     = throws;
}

/* esopt -- parse next option from argument list
 * Arguments:
 *   options: string specifying valid options (e.g., "abc:" means -a, -b, -c arg)
 *           ':' after option letter means that option requires an argument
 * Returns:
 *   option character if valid option found
 *   EOF if no more options
 *   '?' if invalid option (when throw_on_error is FALSE)
 *   ':' if option requires argument but none provided (when throw_on_error is FALSE)
 */
extern int esopt(const char *options)
{   int option_char;
    const char *argument_string;
    const char *option_spec;

    assert(!throw_on_error || usage_message != NULL);
    assert(option_argument == NULL);
    
    if (next_char_position == 0)
    {   if (argument_list == NULL)
            return EOF;
            
        assert(argument_list->term != NULL);
        argument_string = getstr(argument_list->term);
        
        if (*argument_string != OPTION_PREFIX)
            return EOF;
            
        if (argument_string[1] == OPTION_PREFIX && argument_string[2] == '\0')
        {   /* Found "--" end-of-options marker */
            argument_list = argument_list->next;
            return EOF;
        }
        
        next_char_position = 1;
    }
		
    else
    {   assert(argument_list != NULL && argument_list->term != NULL);
        argument_string = getstr(argument_list->term);
    }

    option_char = argument_string[next_char_position++];
    option_spec = strchr(options, option_char);
    
    if (option_spec == NULL)
    {   /* Invalid option */
        const char *error_message = usage_message;
        usage_message             = NULL;
        argument_list             = NULL;
        next_char_position        = 0;
        
        if (throw_on_error)
            fail(invoker_name, "illegal option: -%c -- usage: %s", option_char, error_message);
			
        else
            return OPTION_ERROR;
    }

    if (argument_string[next_char_position] == '\0')
    {   /* End of current argument, move to next */
        next_char_position = 0;
        argument_list = argument_list->next;
    }

    if (option_spec[1] == REQUIRES_ARGUMENT)
    {   /* Option requires an argument */
        if (argument_list == NULL)
        {   const char *error_message = usage_message;

            if (throw_on_error)
                fail(invoker_name,
                     "option -%c expects an argument -- usage: %s",
                     option_char, error_message);
				
            else
                return MISSING_ARG_ERROR;
        }
        
        option_argument = (next_char_position == 0)
                ? argument_list->term
                : mkstr(gcdup(argument_string + next_char_position));
        next_char_position = 0;
        argument_list = argument_list->next;
    }
    
    return option_char;
}

/* esoptarg -- retrieve argument for option that requires one
 * Must be called immediately after esopt() returns an option that requires an argument
 * Returns: Term containing the argument value
 */
extern Term *esoptarg(void)
{   Term *argument_term = option_argument;
    
    assert(argument_term != NULL);
    option_argument = NULL;
    return argument_term;
}

/* esoptend -- finish option parsing and return remaining arguments
 * Returns: List of remaining non-option arguments
 */
extern List *esoptend(void)
{   List *remaining_arguments = argument_list;
    
    argument_list = NULL;
    usage_message = NULL;
    return remaining_arguments;
}
