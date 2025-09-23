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
	const char *topic = NULL;
	
	if (list != NULL) {
		if (list->next != NULL)
			fail("$&help", "usage: help [topic]");
		topic = getstr(list->term);
	}
	
	if (topic == NULL || streq(topic, "")) {
		print("Es Shell Help - Available Topics:\n\n");
		print("ARITHMETIC OPERATORS:\n");
		print("  Infix:     3 + 5, 10 - 3, 4 * 5, 20 / 4, 13 %% 5\n");
		print("  Words:     3 plus 5, 10 minus 3, 4 multiply 5, 20 divide 4, 13 mod 5\n");
		print("  Power:     2 ** 3, 2 power 3\n");
		print("  Min/Max:   5 min 3, 5 max 3\n\n");
		
		print("BITWISE OPERATORS:\n");
		print("  Words:     5 ~and 3, 5 ~or 3, 5 ~xor 3, 8 ~shl 1, 16 ~shr 1\n");
		print("  Alternate: 5 bitwiseand 3, 5 bitwiseor 3, 5 bitwisexor 3\n\n");
		
		print("UNARY OPERATORS:\n");
		print("  Negate:    neg 5, negate 5 (result: -5)\n");
		print("  Positive:  pos 5, positive 5 (result: 5)\n");
		print("  Bitwise:   ~not 5, bitnot 5 (result: -6)\n");
		print("  Absolute:  abs -5 (result: 5)\n\n");
		
		print("PRIMITIVES (use with %%):\n");
		print("  %%addition, %%subtraction, %%multiplication, %%division, %%modulo\n");
		print("  %%pow, %%abs, %%min, %%max, %%count\n");
		print("  %%bitwiseand, %%bitwiseor, %%bitwisexor, %%bitwisenot\n");
		print("  %%bitwiseshiftleft, %%bitwiseshiftright\n\n");
		
		print("EXAMPLES:\n");
		print("  echo <={2 + 3 * 4}     # 14 (precedence: * before +)\n");
		print("  echo <={neg 5 + 3}     # -2 (unary then infix)\n");
		print("  echo <={5 ~and 3}      # 1  (bitwise AND)\n");
		print("  echo <={2 ** 3}        # 8  (power)\n");
		print("  echo <={abs neg 10}    # 10 (absolute of negative)\n\n");
		
		print("For specific help: help arithmetic, help bitwise, help unary, help primitives\n");
	} else if (streq(topic, "arithmetic")) {
		print("ARITHMETIC OPERATORS:\n\n");
		print("Addition:      3 + 5  or  3 plus 5       → 8\n");
		print("Subtraction:   10 - 3  or  10 minus 3     → 7\n");
		print("Multiplication: 4 * 5  or  4 multiply 5   → 20\n");
		print("Division:      20 / 4  or  20 divide 4    → 5\n");
		print("Modulo:        13 %% 5  or  13 mod 5       → 3\n");
		print("Power:         2 ** 3  or  2 power 3      → 8\n");
		print("Minimum:       5 min 3                    → 3\n");
		print("Maximum:       5 max 3                    → 5\n\n");
		print("Precedence: *, /, %%, ** before +, -\n");
		print("Example: 2 + 3 * 4 = 14 (not 20)\n");
	} else if (streq(topic, "bitwise")) {
		print("BITWISE OPERATORS:\n\n");
		print("Bitwise AND:    5 ~and 3  or  5 bitwiseand 3     → 1\n");
		print("Bitwise OR:     5 ~or 3   or  5 bitwiseor 3      → 7\n");
		print("Bitwise XOR:    5 ~xor 3  or  5 bitwisexor 3     → 6\n");
		print("Shift Left:     8 ~shl 1  or  8 shift-left 1     → 16\n");
		print("Shift Right:    16 ~shr 1 or  16 shift-right 1   → 8\n");
		print("Bitwise NOT:    ~not 5    or  bitnot 5           → -6\n\n");
		print("All bitwise operations work on integers.\n");
	} else if (streq(topic, "unary")) {
		print("UNARY OPERATORS:\n\n");
		print("Negate:      neg 5     or  negate 5      → -5\n");
		print("Positive:    pos 5     or  positive 5    → 5\n");
		print("Bitwise NOT: ~not 5    or  bitnot 5      → -6\n");
		print("Absolute:    abs -5                      → 5\n\n");
		print("Unary operators work with infix expressions:\n");
		print("  neg 5 + 3 = -2  (negate 5, then add 3)\n");
	} else if (streq(topic, "primitives")) {
		print("PRIMITIVE FUNCTIONS (use with %%):\n\n");
		print("Arithmetic:\n");
		print("  %%addition 3 5        → 8\n");
		print("  %%subtraction 10 3    → 7\n");
		print("  %%multiplication 4 5  → 20\n");
		print("  %%division 20 4       → 5\n");
		print("  %%modulo 13 5         → 3\n");
		print("  %%pow 2 3             → 8\n");
		print("  %%abs -5              → 5\n");
		print("  %%min 5 3 8           → 3\n");
		print("  %%max 5 3 8           → 8\n");
		print("  %%count a b c d       → 4\n\n");
		print("Bitwise:\n");
		print("  %%bitwiseand 5 3       → 1\n");
		print("  %%bitwiseor 5 3        → 7\n");
		print("  %%bitwisexor 5 3       → 6\n");
		print("  %%bitwisenot 5         → -6\n");
		print("  %%bitwiseshiftleft 8 1 → 16\n");
		print("  %%bitwiseshiftright 16 1 → 8\n");
	} else {
		print("Unknown help topic: %s\n", topic);
		print("Available topics: arithmetic, bitwise, unary, primitives\n");
		print("Use 'help' with no arguments for overview.\n");
	}
	
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
#if HAVE_READLINE
	X(sethistory);
	X(writehistory);
	X(resetterminal);
	X(setmaxhistorylength);
#endif
	return primdict;
}
