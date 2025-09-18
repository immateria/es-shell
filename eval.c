/* eval.c -- evaluation of lists and trees ($Revision: 1.2 $) */

#include "es.h"

/* Evaluation depth tracking for recursion prevention */
unsigned long evaldepth = 0, maxevaldepth = MAXmaxevaldepth;

/* Evaluation flag constants */
#define EVAL_EXIT_ON_FALSE  eval_exitonfalse
#define EVAL_IN_CHILD      eval_inchild

/* failexec -- handle exec failure by calling fn-%exec-failure if defined
 * This is called when execve() fails, allowing user-defined error handling
 */
static Noreturn failexec(char *executable_file, List *argument_list)
{   List *failure_function;
    
    assert(gcisblocked());
    failure_function = varlookup("fn-%exec-failure", NULL);
    
    if (failure_function != NULL)
    {   int saved_errno = errno;
        Ref(List *, combined_list, append(failure_function, mklist(mkstr(executable_file), argument_list)));
        RefAdd(executable_file);
        gcenable();
        RefRemove(executable_file);
        eval(combined_list, NULL, 0);
        RefEnd(combined_list);
        errno = saved_errno;
    }
    
    eprint("%s: %s\n", executable_file, esstrerror(errno));
    esexit(1);
}

/* forkexec -- fork (if necessary) and execute external program
 * Arguments:
 *   executable_file: path to executable
 *   argument_list: command line arguments
 *   inchild: whether we're already in a child process
 * Returns: list containing exit status
 */
extern List *forkexec(char *executable_file, List *argument_list, Boolean inchild)
{   int process_id;
    int exit_status;
    Vector *environment_vars;
    
    gcdisable();
    environment_vars = mkenv();
    process_id = efork(!inchild, FALSE);
    
    if (process_id == 0)
    {   /* Child process - execute the program */
        execve(executable_file, vectorize(argument_list)->vector, environment_vars->vector);
        failexec(executable_file, argument_list);
    }
    
    gcenable();
    exit_status = ewaitfor(process_id);
    
    if ((exit_status & 0xff) == 0)
    {   sigint_newline = FALSE;
        SIGCHK();
        sigint_newline = TRUE;
    }
		
    else
        SIGCHK();
        
    printstatus(0, exit_status);
    return mklist(mkterm(mkstatus(exit_status), NULL), NULL);
}

/* assign -- bind a list of values to a list of variables
 * Implements variable assignment: var1 var2 = value1 value2
 */
static List *assign(Tree *variable_form, Tree *value_form0, Binding *binding0)
{   Ref(List *, result, NULL);

    Ref(Tree    *, value_form,      value_form0);
    Ref(Binding *, binding_context, binding0);
    Ref(List    *, variable_names,  glom(variable_form, binding_context, FALSE));

    if (variable_names == NULL)
        fail("es:assign", "null variable name");

    Ref(List *, value_list, glom(value_form, binding_context, TRUE));
    result = value_list;

    for (; variable_names != NULL; variable_names = variable_names->next)
    {   List *single_value;
        Ref(char *, variable_name, getstr(variable_names->term));
        
        if (value_list == NULL)
            single_value = NULL;
			
        else if (variable_names->next == NULL || value_list->next == NULL)
        {   /* Last variable gets remaining values, or single variable gets single value */
            single_value = value_list;
            value_list = NULL;
        }
			
        else
        {   /* Multiple variables and values - take first value */
            single_value = mklist(value_list->term, NULL);
            value_list = value_list->next;
        }
        vardef(variable_name, binding_context, single_value);
        RefEnd(variable_name);
    }
    RefEnd4(value_list, variable_names, binding_context, value_form);
    RefReturn(result);
}

/* letbindings -- create new binding context with let-bound variables
 * Processes let variable definitions and creates a new binding scope
 */
static Binding *letbindings(Tree *definition0, Binding *outer_binding0,
                           Binding *context0, int UNUSED evalflags)
{   Ref(Binding *, binding_result,     outer_binding0);
    Ref(Binding *, evaluation_context, context0);
    Ref(Tree    *, current_definition, definition0);

    for (; current_definition != NULL; current_definition = current_definition->u[1].p)
    {   assert(current_definition->kind == nList);

        if (current_definition->u[0].p == NULL)
            continue;

        Ref(Tree *, assignment, current_definition->u[0].p);
        assert(assignment->kind == nAssign);
        
        Ref(List *, variable_names, glom(assignment->u[0].p, evaluation_context, FALSE));
        Ref(List *, value_list,     glom(assignment->u[1].p, evaluation_context, TRUE));

        if (variable_names == NULL)
            fail("es:let", "null variable name");

        for (; variable_names != NULL; variable_names = variable_names->next)
        {   List *single_value;
            Ref(char *, variable_name, getstr(variable_names->term));
            
            if (value_list == NULL)
                single_value = NULL;
				
            else if (variable_names->next == NULL || value_list->next == NULL)
            {   single_value = value_list;
                value_list = NULL;
            }
				
            else
            {   single_value = mklist(value_list->term, NULL);
                value_list = value_list->next;
            }
            
            binding_result = mkbinding(variable_name, single_value, binding_result);
            RefEnd(variable_name);
        }

        RefEnd3(value_list, variable_names, assignment);
    }

    RefEnd2(current_definition, evaluation_context);
    RefReturn(binding_result);
}

/* localbind -- recursively convert bindings list into dynamic binding
 * Creates temporary variable bindings using the shell's variable stack
 */
static List *localbind(Binding *dynamic_bindings0, Binding *lexical_bindings0,
                      Tree *body0, int evalflags)
{   if (dynamic_bindings0 == NULL)
        return walk(body0, lexical_bindings0, evalflags);
	
    else
    {   Push variable_stack;
        Ref(List    *, result, NULL);
        Ref(Tree    *, body,             body0);
        Ref(Binding *, dynamic_bindings, dynamic_bindings0);
        Ref(Binding *, lexical_bindings, lexical_bindings0);

        varpush(&variable_stack, dynamic_bindings->name, dynamic_bindings->defn);
        result = localbind(dynamic_bindings->next, lexical_bindings, body, evalflags);
        varpop(&variable_stack);

        RefEnd3(lexical_bindings, dynamic_bindings, body);
        RefReturn(result);
    }
}

/* local -- build one layer of local assignment
 * Implements local variable scope: local var=value { body }
 */
static List *local(Tree *definition, Tree *body0,
                  Binding *bindings0, int evalflags)
{   Ref(List    *, result,          NULL);
    Ref(Tree    *, body,            body0);
    Ref(Binding *, binding_context, bindings0);
    Ref(Binding *, dynamic_bindings,
        reversebindings(letbindings(definition, NULL, binding_context, evalflags)));

    result = localbind(dynamic_bindings, binding_context, body, evalflags);

    RefEnd3(dynamic_bindings, binding_context, body);
    RefReturn(result);
}

/* forloop -- evaluate a for loop with multiple variables and lists
 * Implements: for var1 var2 in list1 list2 { body }
 */
static List *forloop(Tree *definition0, Tree *body0,
                    Binding *binding_context, int evalflags)
{   /* Special marker for variables that share iteration lists */
    static List MULTIPLE_MARKER = { NULL, NULL };

    Ref(List    *, result,        ltrue);
    Ref(Binding *, outer_binding, binding_context);
    Ref(Binding *, loop_bindings, NULL);
    Ref(Tree    *, loop_body,     body0);

    /* Process variable definitions and build iteration bindings */
    Ref(Tree *, current_definition, definition0);
    for (; current_definition != NULL; current_definition = current_definition->u[1].p)
    {   assert(current_definition->kind == nList);
        
        if (current_definition->u[0].p == NULL)
            continue;
            
        Ref(Tree *, assignment, current_definition->u[0].p);
        assert(assignment->kind == nAssign);
        
        Ref(List *, variable_names, glom(assignment->u[0].p, outer_binding, FALSE));
        Ref(List *, iteration_list, glom(assignment->u[1].p, outer_binding, TRUE));
        
        if (variable_names == NULL)
            fail("es:for", "null variable name");
            
        for (; variable_names != NULL; variable_names = variable_names->next)
        {   char *variable_name = getstr(variable_names->term);
            loop_bindings       = mkbinding(variable_name, iteration_list, loop_bindings);
            iteration_list      = &MULTIPLE_MARKER;  /* Subsequent vars share the same list */
        }
        
        RefEnd3(iteration_list, variable_names, assignment);
        SIGCHK();
    }
    
    loop_bindings = reversebindings(loop_bindings);
    RefEnd(current_definition);

    /* Execute the loop iterations */
    ExceptionHandler

        for (;;)
        {   Boolean all_lists_exhausted = TRUE;
            Ref(Binding *, binding_ptr,      outer_binding);
            Ref(Binding *, loop_ptr,         loop_bindings);
            Ref(Binding *, sequence_binding, NULL);
            
            /* Build bindings for current iteration */
            for (; loop_ptr != NULL; loop_ptr = loop_ptr->next)
            {   Ref(List *, current_value, NULL);
                
                if (loop_ptr->defn != &MULTIPLE_MARKER)
                    sequence_binding = loop_ptr;
                    
                assert(sequence_binding != NULL);
                
                if (sequence_binding->defn != NULL)
                {   current_value          = mklist(sequence_binding->defn->term, NULL);
                    sequence_binding->defn = sequence_binding->defn->next;
                    all_lists_exhausted    = FALSE;
                }
                
                binding_ptr = mkbinding(loop_ptr->name, current_value, binding_ptr);
                RefEnd(current_value);
            }
            
            RefEnd2(sequence_binding, loop_ptr);
            
            if (all_lists_exhausted)
            {   RefPop(binding_ptr);
                break;
            }
            
            result = walk(loop_body, binding_ptr, evalflags & EVAL_EXIT_ON_FALSE);
            RefEnd(binding_ptr);
            SIGCHK();
        }

    CatchException(exception)

        if (!termeq(exception->term, "break"))
            throw(exception);
				
        result = exception->next;

    EndExceptionHandler

    RefEnd3(loop_body, loop_bindings, outer_binding);
    RefReturn(result);
}

/* matchpattern -- test if text matches a pattern
 * Returns true/false for pattern matching operations
 */
static List *matchpattern(Tree *subject_form0, Tree *pattern_form0,
                         Binding *binding_context)
{   Boolean match_result;
    List *pattern_list;
    Ref(Binding *, binding_ptr,  binding_context);
    Ref(Tree    *, pattern_form, pattern_form0);
    Ref(List    *, subject_list, glom(subject_form0, binding_ptr, TRUE));
    Ref(StrList *, quote_list,   NULL);
    
    pattern_list = glom2(pattern_form, binding_ptr, &quote_list);
    match_result = listmatch(subject_list, pattern_list, quote_list);
    
    RefEnd4(quote_list, subject_list, pattern_form, binding_ptr);
    return match_result ? ltrue : lfalse;
}

/* extractpattern -- like matchpattern but returns the matched portions
 * Used for pattern extraction operations
 */
static List *extractpattern(Tree *subject_form0, Tree *pattern_form0,
                           Binding *binding_context)
{   List *pattern_list;
    Ref(List    *, result,       NULL);
    Ref(Binding *, binding_ptr,  binding_context);
    Ref(Tree    *, pattern_form, pattern_form0);
    Ref(List    *, subject_list, glom(subject_form0, binding_ptr, TRUE));
    Ref(StrList *, quote_list,   NULL);
    
    pattern_list = glom2(pattern_form, binding_ptr, &quote_list);
    result       = (List *)extractmatches(subject_list, pattern_list, quote_list);
    
    RefEnd4(quote_list, subject_list, pattern_form, binding_ptr);
    RefReturn(result);
}

/* walk -- walk through a parse tree, evaluating nodes
 * This is the main tree evaluation dispatcher that handles all node types
 */
extern List *walk(Tree *tree0, Binding *binding0, int flags)
{   Tree    *volatile current_tree    = tree0;
    Binding *volatile binding_context = binding0;

    SIGCHK();

top:
    if (current_tree == NULL)
        return ltrue;

    switch (current_tree->kind)
    {
        /* Expression nodes that need glomming (expansion) first */
        case nConcat: case nList: case nQword: case nVar: case nVarsub:
        case nWord: case nThunk: case nLambda: case nCall: case nPrim:
        {   List *expanded_list;
            Ref(Binding *, binding_ptr, binding_context);
            expanded_list   = glom(current_tree, binding_context, TRUE);
            binding_context = binding_ptr;
            RefEnd(binding_ptr);
	
            return eval(expanded_list, binding_context, flags);
        }

        case nAssign:
            return assign(current_tree->u[0].p, current_tree->u[1].p, binding_context);

        /* Let bindings and closures create new binding contexts */
        case nLet: case nClosure:
            Ref(Tree *, body, current_tree->u[1].p);
            binding_context = letbindings(current_tree->u[0].p, binding_context, binding_context, flags);
            current_tree    = body;
            RefEnd(body);
			
            goto top;  /* Tail call optimization */

        case nLocal:
            return local(current_tree->u[0].p, current_tree->u[1].p, binding_context, flags);

        case nFor:
            return forloop(current_tree->u[0].p, current_tree->u[1].p, binding_context, flags);

        case nMatch:
            return matchpattern(current_tree->u[0].p, current_tree->u[1].p, binding_context);

        case nExtract:
            return extractpattern(current_tree->u[0].p, current_tree->u[1].p, binding_context);

        default:
            panic("walk: bad node kind %d", current_tree->kind);
    }
    NOTREACHED;
}

/* bindargs -- bind argument list to lambda parameters
 * Creates variable bindings for function parameters
 */
extern Binding *bindargs(Tree *parameter_list, List *argument_list, Binding *outer_binding)
{   if (parameter_list == NULL)
        return mkbinding("*", argument_list, outer_binding);

    gcdisable();

    for (; parameter_list != NULL; parameter_list = parameter_list->u[1].p)
    {   Tree *parameter;
        List *parameter_value;
        
        assert(parameter_list->kind == nList);
        parameter = parameter_list->u[0].p;
        assert(parameter->kind == nWord || parameter->kind == nQword);
        
        if (argument_list == NULL)
            parameter_value = NULL;
        else if (parameter_list->u[1].p == NULL || argument_list->next == NULL)
        {   /* Last parameter gets remaining arguments */
            parameter_value = argument_list;
            argument_list = NULL;
        }

        else
        {   /* Take single argument for this parameter */
            parameter_value = mklist(argument_list->term, NULL);
            argument_list = argument_list->next;
        }
        
        outer_binding = mkbinding(parameter->u[0].s, parameter_value, outer_binding);
    }

    Ref(Binding *, result, outer_binding);
    gcenable();
    RefReturn(result);
}

/* pathsearch -- evaluate fn %pathsearch with given argument
 * Used for command path resolution
 */
extern List *pathsearch(Term *search_term)
{   List *command_list;
    Ref(List *, search_function, NULL);
    
    search_function = varlookup("fn-%pathsearch", NULL);
    if (search_function == NULL)
        fail("es:pathsearch", "%E: fn %%pathsearch undefined", search_term);
        
    command_list = mklist(search_term, NULL);
    command_list = append(search_function, command_list);
    RefEnd(search_function);
    
    return eval(command_list, NULL, 0);
}

/* eval -- evaluate a list, producing a result list
 * This is the main evaluation engine that handles function calls, primitives, and external commands
 */
extern List *eval(List *list0, Binding *binding0, int flags)
{   Closure *volatile closure_ptr;
    List    *function_definition;

    if (++evaldepth >= maxevaldepth)
        fail("es:eval", "max-eval-depth exceeded");

    Ref(List    *, current_list,    list0);
    Ref(Binding *, binding_context, binding0);
    Ref(char    *, function_name,   NULL);

restart:
    SIGCHK();
    
    if (current_list == NULL)
    {   RefPop3(function_name, binding_context, current_list);
        --evaldepth;
        return ltrue;
    }
    
    assert(current_list->term != NULL);

    /* Check if first element is a closure (function, primitive, etc.) */
    if ((closure_ptr = getclosure(current_list->term)) != NULL)
    {   switch (closure_ptr->tree->kind)
        {
            case nPrim:
                /* Built-in primitive function */
                assert(closure_ptr->binding == NULL);
                current_list = prim(closure_ptr->tree->u[0].s, current_list->next, binding_context, flags);
                break;
                
            case nThunk:
                /* Unevaluated expression */
                current_list = walk(closure_ptr->tree->u[0].p, closure_ptr->binding, flags);
                break;
                
            case nLambda:
                /* User-defined function */
                ExceptionHandler

                    Push function_context;
                    Ref(Tree    *, lambda_tree, closure_ptr->tree);
                    Ref(Binding *, parameter_bindings,
                        bindargs(lambda_tree->u[0].p, current_list->next, closure_ptr->binding));
                        
                    if (function_name != NULL)
                        varpush(&function_context, "0",
                               mklist(mkterm(function_name, NULL), NULL));
                               
                    current_list = walk(lambda_tree->u[1].p, parameter_bindings, flags);
                    
                    if (function_name != NULL)
                        varpop(&function_context);
                        
                    RefEnd2(parameter_bindings, lambda_tree);

                CatchException(exception)

                    if (termeq(exception->term, "return"))
                    {   current_list = exception->next;
                        goto done;
                    }
			
                    throw(exception);

                EndExceptionHandler
                break;
                
            case nList:
                /* List expansion */
                Ref(List *, expanded_list, glom(closure_ptr->tree, closure_ptr->binding, TRUE));
                current_list = append(expanded_list, current_list->next);
                RefEnd(expanded_list);
				
                goto restart;
                
            case nConcat:
                /* Handle concatenated primitive names (error case) */
                Ref(Tree *, concat_tree, closure_ptr->tree);
                while (concat_tree->kind == nConcat)
                    concat_tree = concat_tree->u[0].p;
				
                if (concat_tree->kind == nPrim)
                    fail("es:eval", "invalid primitive name: %T", closure_ptr->tree);
				
                RefEnd(concat_tree);
                FALLTHROUGH;
                
            default:
                panic("eval: bad closure node kind %d", closure_ptr->tree->kind);
        }
        goto done;
    }

    /* Not a closure - look up as function name or external command */
    /* Note: this logic is duplicated in $&whatis primitive */

    Ref(char *, command_name, getstr(current_list->term));
    function_definition = varlookup2("fn-", command_name, binding_context);
    
    if (function_definition != NULL)
    {   /* Found user-defined function */
        function_name = command_name;
        current_list  = append(function_definition, current_list->next);
        RefPop(command_name);
		
        goto restart;
    }
    
    if (isabsolute(command_name))
    {   /* Absolute path to executable */
        char *executable_error = checkexecutable(command_name);
        if (executable_error != NULL)
            fail("$&whatis", "%s: %s", command_name, executable_error);
            
        if (function_name != NULL)
        {   Term *function_term = mkstr(function_name);
            current_list = mklist(function_term, current_list->next);
        }
        
        current_list = forkexec(command_name, current_list, flags & EVAL_IN_CHILD);
        RefPop(command_name);
        goto done;
    }
    
    RefEnd(command_name);

    /* Search PATH for external command */
    function_definition = pathsearch(current_list->term);
    
    if (function_definition != NULL && function_definition->next == NULL
        && (closure_ptr = getclosure(function_definition->term)) == NULL)
    {   /* Found external executable */
        char *executable_name = getstr(function_definition->term);
        current_list = forkexec(executable_name, current_list, flags & EVAL_IN_CHILD);
		
        goto done;
    }

    /* Treat as function call */
    if (function_definition != NULL)
        function_name = getstr(current_list->term);
        
    current_list = append(function_definition, current_list->next);
    goto restart;

done:
    --evaldepth;
    
    if ((flags & EVAL_EXIT_ON_FALSE) && !istrue(current_list))
        esexit(exitstatus(current_list));
        
    RefEnd2(function_name, binding_context);
    RefReturn(current_list);
}

/* eval1 -- evaluate a single term, producing a list
 * Convenience function for evaluating single terms
 */
extern List *eval1(Term *term, int flags)
{ return eval(mklist(term, NULL), NULL, flags); }
