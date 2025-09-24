/* prim-etc.c -- miscellaneous primitives ($Revision: 1.2 $) */

#define	REQUIRE_PWD	1

#include "es.h"
#include "prim.h"

PRIM(result) {
	return list;
}

PRIM(echo) {
	const char *eol = "\n";
	if (list != NULL) {
		if (termeq(list->term, "-n")) {
			eol = "";
			list = list->next;
		} else if (termeq(list->term, "--"))
			list = list->next;
	}
	
	/* Handle empty list case */
	if (list == NULL) {
		print("%s", eol);
		return ltrue;
	}
	
	/* Print each list item using print() for proper redirection support */
	List *lp = list;
	while (lp != NULL) {
		char *str = getstr(lp->term);
		if (str != NULL) {
			print("%s", str);
		}
		if (lp->next != NULL)
			print(" ");
		lp = lp->next;
	}
	print("%s", eol);
	return ltrue;
}


PRIM(setnoexport) {
	Ref(List *, lp, list);
	setnoexport(lp);
	RefReturn(lp);
}

PRIM(version) {
	return mklist(mkstr((char *) version), NULL);
}

PRIM(exec) {
	return eval(list, NULL, evalflags | eval_inchild);
}

PRIM(dot) {
	int c, fd;
	Push zero, star;
	volatile int runflags = (evalflags & eval_inchild);
	const char * const usage = ". [-einvx] file [arg ...]";

	esoptbegin(list, "$&dot", usage, TRUE);
	while ((c = esopt("einvx")) != EOF)
		switch (c) {
		case 'e':	runflags |= eval_exitonfalse;	break;
		case 'i':	runflags |= run_interactive;	break;
		case 'n':	runflags |= run_noexec;		break;
		case 'v':	runflags |= run_echoinput;	break;
		case 'x':	runflags |= run_printcmds;	break;
		}

	Ref(List *, result, NULL);
	Ref(List *, lp, esoptend());
	if (lp == NULL)
		fail("$&dot", "usage: %s", usage);

	Ref(char *, file, getstr(lp->term));
	lp = lp->next;
	fd = eopen(file, oOpen);
	if (fd == -1)
		fail("$&dot", "%s: %s", file, esstrerror(errno));

	varpush(&star, "*", lp);
	varpush(&zero, "0", mklist(mkstr(file), NULL));

	result = runfd(fd, file, runflags);

	varpop(&zero);
	varpop(&star);
	RefEnd2(file, lp);
	RefReturn(result);
}

PRIM(flatten) {
	char *sep;
	if (list == NULL)
		fail("$&flatten", "usage: %%flatten separator [args ...]");
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = mklist(mkstr(str("%L", lp->next, sep)), NULL);
	RefReturn(lp);
}

PRIM(whatis) {
	/* the logic in here is duplicated in eval() */
	if (list == NULL || list->next != NULL)
		fail("$&whatis", "usage: $&whatis program");
	Ref(Term *, term, list->term);
	if (getclosure(term) == NULL) {
		List *fn;
		Ref(char *, prog, getstr(term));
		assert(prog != NULL);
		fn = varlookup2("fn-", prog, binding);
		if (fn != NULL)
			list = fn;
		else {
			if (isabsolute(prog)) {
				char *error = checkexecutable(prog);
				if (error != NULL)
					fail("$&whatis", "%s: %s", prog, error);
			} else
				list = pathsearch(term);
		}
		RefEnd(prog);
	}
	RefEnd(term);
	return list;
}

PRIM(split) {
	char *sep;
	if (list == NULL)
		fail("$&split", "usage: %%split separator [args ...]");
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = fsplit(sep, lp->next, TRUE);
	RefReturn(lp);
}

PRIM(fsplit) {
	char *sep;
	if (list == NULL)
		fail("$&fsplit", "usage: %%fsplit separator [args ...]");
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = fsplit(sep, lp->next, FALSE);
	RefReturn(lp);
}

PRIM(var) {
	Term *term;
	if (list == NULL)
		return NULL;
	Ref(List *, rest, list->next);
	Ref(char *, name, getstr(list->term));
	Ref(List *, defn, varlookup(name, NULL));
	rest = prim_var(rest, NULL, evalflags);
	term = mkstr(str("%S = %#L", name, defn, " "));
	list = mklist(term, rest);
	RefEnd3(defn, name, rest);
	return list;
}

static void loginput(char *input) {
	char *c;
	List *fn = varlookup("fn-%write-history", NULL);
	if (!isinteractive() || !isfromfd() || fn == NULL)
		return;
	for (c = input;; c++)
		switch (*c) {
		case '#': case '\n': return;
		case ' ': case '\t': break;
		default: goto writeit;
		}
writeit:
	gcdisable();
	Ref(List *, list, append(fn, mklist(mkstr(input), NULL)));
	gcenable();
	eval(list, NULL, 0);
	RefEnd(list);
}

PRIM(parse) {
	List *result;
	Ref(char *, prompt1, NULL);
	Ref(char *, prompt2, NULL);
	Ref(List *, lp, list);
	if (lp != NULL) {
		prompt1 = getstr(lp->term);
		if ((lp = lp->next) != NULL)
			prompt2 = getstr(lp->term);
	}
	RefEnd(lp);
	newhistbuffer();

	Ref(Tree *, tree, NULL);
	ExceptionHandler
		tree = parse(prompt1, prompt2);
	CatchException (ex)
		Ref(List *, e, ex);
		loginput(dumphistbuffer());
		throw(e);
		RefEnd(e);
	EndExceptionHandler

	loginput(dumphistbuffer());
	result = (tree == NULL)
		   ? NULL
		   : mklist(mkterm(NULL, mkclosure(gcmk(nThunk, tree), NULL)),
			    NULL);
	RefEnd3(tree, prompt2, prompt1);
	return result;
}

PRIM(exitonfalse) {
	return eval(list, NULL, evalflags | eval_exitonfalse);
}

PRIM(batchloop) {
	Ref(List *, result, ltrue);
	Ref(List *, dispatch, NULL);

	SIGCHK();

	ExceptionHandler

		for (;;) {
			List *parser, *cmd;
			parser = varlookup("fn-%parse", NULL);
			cmd = (parser == NULL)
					? prim("parse", NULL, NULL, 0)
					: eval(parser, NULL, 0);
			SIGCHK();
			dispatch = varlookup("fn-%dispatch", NULL);
			if (cmd != NULL) {
				if (dispatch != NULL)
					cmd = append(dispatch, cmd);
				result = eval(cmd, NULL, evalflags);
				SIGCHK();
			}
		}

	CatchException (e)

		if (!termeq(e->term, "eof"))
			throw(e);
		RefEnd(dispatch);
		if (result == ltrue)
			result = ltrue;
		RefReturn(result);

	EndExceptionHandler
}

PRIM(collect) {
	gc();
	return ltrue;
}

PRIM(home) {
	struct passwd *pw;
	if (list == NULL)
		return varlookup("home", NULL);
	if (list->next != NULL)
		fail("$&home", "usage: %%home [user]");
	pw = getpwnam(getstr(list->term));
	return (pw == NULL) ? NULL : mklist(mkstr(gcdup(pw->pw_dir)), NULL);
}

PRIM(vars) {
	return listvars(FALSE);
}

PRIM(internals) {
	return listvars(TRUE);
}

PRIM(isinteractive) {
	return isinteractive() ? ltrue : lfalse;
}

#ifdef noreturn
#undef noreturn
#endif
PRIM(noreturn) {
	if (list == NULL)
		fail("$&noreturn", "usage: $&noreturn lambda args ...");
	Ref(List *, lp, list);
	Ref(Closure *, closure, getclosure(lp->term));
	if (closure == NULL || closure->tree->kind != nLambda)
		fail("$&noreturn", "$&noreturn: %E is not a lambda", lp->term);
	Ref(Tree *, tree, closure->tree);
	Ref(Binding *, context, bindargs(tree->u[0].p, lp->next, closure->binding));
	lp = walk(tree->u[1].p, context, evalflags);
	RefEnd3(context, tree, closure);
	RefReturn(lp);
}

PRIM(setmaxevaldepth) {
	char *s;
	long n;
	if (list == NULL) {
		maxevaldepth = MAXmaxevaldepth;
		return NULL;
	}
	if (list->next != NULL)
		fail("$&setmaxevaldepth", "usage: $&setmaxevaldepth [limit]");
	Ref(List *, lp, list);
	n = strtol(getstr(lp->term), &s, 0);
	if (n < 0 || (s != NULL && *s != '\0'))
		fail("$&setmaxevaldepth", "max-eval-depth must be set to a positive integer");
	if (n < MINmaxevaldepth)
		n = (n == 0) ? MAXmaxevaldepth : MINmaxevaldepth;
	maxevaldepth = n;
	RefReturn(lp);
}

#if HAVE_READLINE
PRIM(sethistory) {
	if (list == NULL) {
		sethistory(NULL);
		return NULL;
	}
	Ref(List *, lp, list);
	sethistory(getstr(lp->term));
	RefReturn(lp);
}

PRIM(writehistory) {
	if (list == NULL || list->next != NULL)
		fail("$&writehistory", "usage: $&writehistory command");
	loghistory(getstr(list->term));
	return NULL;
}

PRIM(setmaxhistorylength) {
	char *s;
	int n;
	if (list == NULL) {
		setmaxhistorylength(-1); /* unlimited */
		return NULL;
	}
	if (list->next != NULL)
		fail("$&setmaxhistorylength", "usage: $&setmaxhistorylength [limit]");
	Ref(List *, lp, list);
	n = (int)strtol(getstr(lp->term), &s, 0);
	if (n < 0 || (s != NULL && *s != '\0'))
		fail("$&setmaxhistorylength", "max-history-length must be set to a positive integer");
	setmaxhistorylength(n);
	RefReturn(lp);
}

PRIM(resetterminal) {
	resetterminal = TRUE;
	return ltrue;
}
#endif

PRIM(help) {
    char *topic = NULL;
    
    // Get topic if provided
    if (list != NULL) {
        if (list->next != NULL)
            fail("$&help", "usage: help [topic]");
        topic = getstr(list->term);
    }
    
    if (topic == NULL || streq(topic, "")) {
        // General help overview
        print("ES Shell Help System\n");
        print("===================\n\n");
        print("ES Shell is a functional shell with modern syntax enhancements.\n\n");
        print("Available help topics:\n");
        print("  help arithmetic     - ${...} expressions and math operations\n");
        print("  help redirection    - New -> and <- operators\n");
        print("  help variables      - Variable assignment and usage\n");
        print("  help functions      - Function definition and usage\n");
        print("  help primitives     - Built-in primitive operations\n");
        print("  help syntax         - Basic ES Shell syntax\n");
        
        print("\nGeneral commands:\n");
        print("  help <topic>        - Get help on specific topic\n");
        print("  discover            - Discover available commands\n");
        print("  examples            - Show usage examples\n");
        print("  echo $&primitives   - List all primitive operations\n");
        print("\nPhase 1 Features (NEW):\n");
        print("  ${5 plus 3}         - Expression evaluation (→ 8)\n");
        print("  echo data -> file   - Arrow redirection\n");
        print("  if {${x} > 5} {...} - Comparisons in conditionals\n");
        
        return ltrue;
    }
    
    // Handle specific topics
    if (streq(topic, "arithmetic")) {
        print("Arithmetic operations and expressions\n");
        print("====================================\n\n");
        print("ES Shell supports arithmetic with the new ${...} syntax:\n\n");
        print("Basic operators:\n");
        print("  plus, minus, times, div, mod - word-based operators\n");
        print("  >, <, >=, <=, ==, != - comparison operators\n\n");
        print("Operator precedence (highest to lowest):\n");
        print("  times, div, mod\n");
        print("  plus, minus\n");
        print("  comparisons\n\n");
        print("Examples:\n");
        print("  echo ${5 plus 3}        # → 8\n");
        print("  x = 5\n");
        print("  echo ${x times 2}       # → 10\n");
        print("  echo ${2 plus 3 times 4}  # → 14 (proper precedence)\n");
        
    } else if (streq(topic, "redirection")) {
        print("New arrow-based redirection operators\n");
        print("====================================\n\n");
        print("ES Shell uses arrow operators for redirection:\n\n");
        print("Input/Output:\n");
        print("  <-      input redirection (replaces <)\n");
        print("  ->      output redirection (replaces >)\n");
        print("  ->>     append redirection\n\n");
        print("Examples:\n");
        print("  echo \"hello\" -> output.txt\n");
        print("  cat <- input.txt\n");
        print("  echo \"line\" ->> log.txt\n");
        
    } else if (streq(topic, "variables")) {
        print("Variable assignment and usage\n");
        print("=============================\n\n");
        print("ES Shell variables are functional and immutable:\n\n");
        print("Assignment:\n");
        print("  var = value          # simple assignment\n");
        print("  list = (a b c)       # list assignment\n\n");
        print("Access:\n");
        print("  $var                 # variable value\n");
        print("  $#var                # count elements\n\n");
        print("Examples:\n");
        print("  name = Alice\n");
        print("  echo \"Hello $name\"      # → Hello Alice\n");
        print("  count = 5\n");
        print("  total = ${count times 10} # → 50\n");
        
    } else if (streq(topic, "functions")) {
        print("Function definition and usage\n");
        print("=============================\n\n");
        print("ES Shell functions are first-class values:\n\n");
        print("Definition:\n");
        print("  fn name params { body }\n\n");
        print("Examples:\n");
        print("  fn greet name { echo \"Hello $name!\" }\n");
        print("  greet Alice               # → Hello Alice!\n");
        print("  fn double x { echo ${x times 2} }\n");
        print("  double 7                  # → 14\n");
        
    } else if (streq(topic, "primitives")) {
        print("Built-in primitive operations\n");
        print("============================\n\n");
        print("ES Shell provides built-in primitives with %% prefix:\n\n");
        print("Arithmetic:\n");
        print("  %%addition, %%subtraction, %%multiplication, %%division\n");
        print("  %%count, %%split, %%flatten\n\n");
        print("Examples:\n");
        print("  echo %%addition 5 3        # → 8\n");
        print("  echo %%count (a b c)       # → 3\n\n");
        print("See all: echo $&primitives\n");
        
    } else if (streq(topic, "syntax")) {
        print("Basic ES Shell syntax overview\n");
        print("==============================\n\n");
        print("Commands:\n");
        print("  command args              # simple command\n");
        print("  {command args}            # grouped command\n\n");
        print("Control flow:\n");
        print("  if {condition} {then} {else}\n");
        print("  for (var = list) { body }\n\n");
        print("Expressions (NEW):\n");
        print("  ${expression}             # evaluate expression\n");
        print("  result = ${5 times 6}     # → 30\n");
        
    } else {
        // Fallback for unknown topics or legacy math topics
        if (streq(topic, "bitwise") || streq(topic, "unary")) {
            print("Legacy math topic. Try: help arithmetic\n");
            print("For comprehensive help: help (no arguments)\n");
        } else {
            print("Help topic '%s' not found.\n\n", topic);
            print("Available topics: arithmetic, redirection, variables, functions, primitives, syntax\n");
            print("Try: help (without arguments) for overview\n");
        }
        return lfalse;
    }
    
    return ltrue;
}

PRIM(discover) {
    if (list != NULL) {
        fail("$&discover", "usage: discover");
    }
    
    print("ES Shell Command Discovery\n");
    print("=========================\n\n");
    
    print("Built-in commands:\n");
    print("  exit, quit              - Exit the shell\n");
    print("  cd, pwd                 - Directory navigation\n");
    print("  set, unset              - Environment variables\n");
    print("  echo, printf            - Output text\n");
    print("  cat, ls, grep           - File operations\n\n");
    
    print("Control structures:\n");
    print("  if ... then ... else    - Conditionals\n");
    print("  for ... in ...          - Loops\n");
    print("  fn ... { ... }          - Function definition\n\n");
    
    print("ES Shell features:\n");
    print("  ${...}                  - Expression evaluation\n");
    print("  -> <-                   - Arrow redirection\n");
    print("  help, examples          - Documentation\n\n");
    
    print("Primitive functions (use with %%):\n");
    print("  %%addition, %%count       - Math operations\n");
    print("  %%split, %%flatten        - List operations\n");
    print("  Use: echo $&primitives  - List all primitives\n\n");
    
    print("For detailed help: help <topic>\n");
    
    return ltrue;
}

PRIM(examples) {
    if (list != NULL) {
        fail("$&examples", "usage: examples");
    }
    
    print("ES Shell Usage Examples\n");
    print("======================\n\n");
    
    print("Basic arithmetic:\n");
    print("  echo ${5 plus 3}         → 8\n");
    print("  echo ${10 minus 4}       → 6\n");
    print("  result = ${3 times 7}    → result holds 21\n\n");
    
    print("Comparisons:\n");
    print("  if {${x} > 5} {echo \"big\"}\n");
    print("  if {${age} >= 18} {echo \"adult\"}\n\n");
    
    print("Variables and lists:\n");
    print("  name = Alice\n");
    print("  files = (*.txt)          → glob expansion\n");
    print("  echo $#files             → count of files\n\n");
    
    print("Redirection (new syntax):\n");
    print("  echo \"Hello\" -> greeting.txt\n");
    print("  cat <- input.txt\n");
    print("  date ->> log.txt         → append\n\n");
    
    print("Functions:\n");
    print("  fn double x { echo ${x times 2} }\n");
    print("  double 15                → 30\n\n");
    
    print("Primitives:\n");
    print("  echo %%addition 8 7       → 15\n");
    print("  echo %%count a b c        → 3\n\n");
    
    print("Complex expressions:\n");
    print("  total = ${base plus tax times rate}\n");
    print("  valid = {${score} >= ${threshold}}\n\n");
    
    return ltrue;
}


/*
 * initialization
 */

extern Dict *initprims_etc(Dict *primdict) {
        X(echo);
        X(version);
        X(exec);
        X(dot);
        X(flatten);
        X(whatis);
        X(split);
	X(fsplit);
	X(var);
	X(parse);
	X(batchloop);
	X(collect);
	X(home);
	X(setnoexport);
	X(vars);
	X(internals);
	X(result);
	X(isinteractive);
	X(exitonfalse);
	X(noreturn);
	X(setmaxevaldepth);
	X(help);
	X(discover);
	X(examples);
#if HAVE_READLINE
	X(sethistory);
	X(writehistory);
	X(resetterminal);
	X(setmaxhistorylength);
#endif
	return primdict;
}
