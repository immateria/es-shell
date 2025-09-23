/* parse.y -- grammar for es ($Revision: 1.2 $) */

%{
/* Some yaccs insist on including stdlib.h */
#include "es.h"
#include "input.h"
#include "syntax.h"

static Tree *arithword(Tree *t) {
       if (t != NULL && t->kind == nWord) {
               char *end;
               strtol(t->u[0].s, &end, 10);
               
               /* If it's a pure number, return as-is for arithmetic use */
               if (*end == '\0') {
                       return t;
               }
               
               /* CONSERVATIVE APPROACH: Don't auto-convert words to variables */
               /* This fixes the echo problem. If arithmetic contexts need */
               /* variable expansion, they should use explicit $var syntax */
               /* or we need a more sophisticated context-aware approach */
               
               return t;  /* Keep all non-numeric words as literals */
       }
       return t;
}
%}

%token <str>	WORD QWORD
%token		LOCAL LET FOR CLOSURE FN
%token <tree>	REDIR DUP
%token          ANDAND BACKBACK BBFLAT BFLAT EXTRACT CALL COUNT FLAT OROR PRIM SUB
%token          LT GT LE GE EQ NE  /* Comparison operators */
%token		EXPR_CALL  /* ${...} expression evaluation */
%token		LARROW RARROW  /* <- and -> for redirection */
%token		FLARROW FRARROW  /* <-! and ->! for forced redirection */
%token		HEREDOC_NEW  /* <--< for new heredoc syntax */
%token		RW_ARROW  /* <-> for read-write */
%token		OA_ARROW  /* <->> for open-append */
%token		OC_ARROW  /* ->-< for open-create */
%token		APPEND_ARROW  /* ->> for append */
%token		APPEND_CREATE  /* ->>< for append with create */
%token		NL ENDFILE ERROR
%token		MATCH

%left           '^' '='
%left		MATCH LOCAL LET FOR CLOSURE ')'
%left		ANDAND OROR NL
%left		'!'
%left		LT GT LE GE EQ NE  /* Comparison operators */
%left <tree>	PIPE
%right		'$' EXPR_CALL
%left		SUB

%union {
	Tree *tree;
	char *str;
	NodeKind kind;
}

%type <str>	keyword
%type <tree>	body cmd cmdsa cmdsan comword first fn line word param assign
		args binding bindings params nlwords words simple redir sword
		cases case
%type <kind>	binder

%start es

%%

es	: line end		{ parsetree = $1; YYACCEPT; }
	| error end		{ yyerrok; parsetree = NULL; YYABORT; }

end	: NL			{ if (!readheredocs(FALSE)) YYABORT; }
	| ENDFILE		{ if (!readheredocs(TRUE)) YYABORT; }

line	: cmd			{ $$ = $1; }
	| cmdsa line		{ $$ = mkseq("%seq", $1, $2); }

body	: cmd			{ $$ = $1; }
	| cmdsan body		{ $$ = mkseq("%seq", $1, $2); }

cmdsa	: cmd ';'		{ $$ = $1; }
	| cmd '&'		{ $$ = prefix("%background", mk(nList, thunkify($1), NULL)); }

cmdsan	: cmdsa			{ $$ = $1; }
	| cmd NL		{ $$ = $1; if (!readheredocs(FALSE)) YYABORT; }

cmd	:		%prec LET		{ $$ = NULL; }
	| simple				{ $$ = redirect($1); if ($$ == &errornode) YYABORT; }
	| redir cmd	%prec '!'		{ $$ = redirect(mk(nRedir, $1, $2)); if ($$ == &errornode) YYABORT; }
	| first assign				{ $$ = mk(nAssign, $1, $2); }
	| fn					{ $$ = $1; }
	| binder nl '(' bindings ')' nl cmd	{ $$ = mk($1, $4, $7); }
	| cmd ANDAND nl cmd			{ $$ = mkseq("%and", $1, $4); }
	| cmd OROR nl cmd			{ $$ = mkseq("%or", $1, $4); }
 	| cmd PIPE nl cmd			{ $$ = mkpipe($1, $2->u[0].i, $2->u[1].i, $4); }
	| '!' caret cmd				{ $$ = prefix("%not", mk(nList, thunkify($3), NULL)); }
	| '~' word words			{ $$ = mk(nMatch, $2, $3); }
	| EXTRACT word words			{ $$ = mk(nExtract, $2, $3); }
	| MATCH word nl '(' cases ')'		{ $$ = mkmatch($2, $5); }

cases	: case				{ $$ = treecons($1, NULL); }
	| cases ';' case		{ $$ = treeconsend($1, $3); }
	| cases NL case			{ $$ = treeconsend($1, $3); }

case	:				{ $$ = NULL; }
	| word first			{ $$ = mk(nMatch, $1, thunkify($2)); }

simple	: first				{ Tree *expr = rewriteinfix($1, NULL); $$ = (expr != NULL) ? expr : treecons($1, NULL); }
	| first args			{ Tree *expr = rewriteinfix($1, $2); $$ = (expr != NULL) ? expr : firstprepend($1, $2); }

args	: word				{ $$ = treecons($1, NULL); }
	| redir				{ $$ = redirappend(NULL, $1); }
	| args word			{ $$ = treeconsend($1, $2); }
	| args redir			{ $$ = redirappend($1, $2); }

redir	: DUP				{ $$ = $1; }
	| REDIR word			{ $$ = mkredir($1, $2); }
	| LARROW word			{ $$ = mkredir(mkredircmd("%open", 0), $2); }  /* <- input redirection */
	| RARROW word			{ $$ = mkredir(mkredircmd("%create", 1), $2); }  /* -> output redirection */
	| FLARROW word			{ $$ = mkredir(mkredircmd("%open-force", 0), $2); }  /* <-! forced input */
	| FRARROW word			{ $$ = mkredir(mkredircmd("%create-force", 1), $2); }  /* ->! forced output */
	| HEREDOC_NEW word		{ $$ = mkredir(mkredircmd("%heredoc", 0), $2); }  /* <--< heredoc */
	| RW_ARROW word			{ $$ = mkredir(mkredircmd("%open-write", 0), $2); }  /* <-> read-write */
	| OA_ARROW word			{ $$ = mkredir(mkredircmd("%open-append", 0), $2); }  /* <->> open-append */
	| OC_ARROW word			{ $$ = mkredir(mkredircmd("%open-create", 1), $2); }  /* ->-< open-create */
	| APPEND_ARROW word		{ $$ = mkredir(mkredircmd("%append", 1), $2); }  /* ->> append */
	| APPEND_CREATE word		{ $$ = mkredir(mkredircmd("%open-append", 1), $2); }  /* ->>< append with create */

bindings: binding			{ $$ = treecons($1, NULL); }
	| bindings ';' binding		{ $$ = treeconsend($1, $3); }
	| bindings NL binding		{ $$ = treeconsend($1, $3); }

binding	:				{ $$ = NULL; }
	| fn				{ $$ = $1; }
	| first assign			{ $$ = mk(nAssign, $1, $2); }

assign	: caret '=' caret words		{ $$ = $4; }

fn	: FN word params '{' body '}'	{ $$ = fnassign($2, mklambda($3, $5)); }
	| FN word			{ $$ = fnassign($2, NULL); }

first	: comword			{ $$ = $1; }
	| first '^' sword		{ $$ = mk(nConcat, $1, $3); }

sword	: comword			{ $$ = $1; }
	| keyword			{ $$ = mk(nWord, $1); }

word    : sword                         { $$ = arithword($1); }
        | word '^' sword                { $$ = mk(nConcat, $1, $3); }

comword : param				{ $$ = $1; }
	| '(' nlwords ')'		{ $$ = mk(nList, $2, NULL); }
	| '{' body '}'			{ $$ = thunkify($2); }
	| '@' params '{' body '}'	{ $$ = mklambda($2, $4); }
	| '$' sword			{ $$ = mk(nVar, $2); }
	| '$' sword SUB words ')'	{ $$ = mk(nVarsub, $2, $4); }
	| EXPR_CALL body '}'		{ $$ = mk(nCall, treecons(thunkify($2), NULL)); }  /* ${...} expression evaluation */
	| CALL sword			{ $$ = mk(nCall, $2); }
	| COUNT sword			{ $$ = mk(nCall, prefix("%count", treecons(mk(nVar, $2), NULL))); }
	| FLAT sword			{ $$ = flatten(mk(nVar, $2), " "); }
	| PRIM WORD			{ $$ = mk(nPrim, $2); }
	| '`' sword			{ $$ = backquote(mk(nVar, mk(nWord, "ifs")), $2); }
	| BFLAT sword			{ $$ = flatten(backquote(mk(nVar, mk(nWord, "ifs")), $2), " "); }
	| BACKBACK word	sword		{ $$ = backquote($2, $3); }
	| BBFLAT word sword             { $$ = flatten(backquote($2, $3), " " ); }

param	: WORD				{ $$ = mk(nWord, $1); }
	| QWORD				{ $$ = mk(nQword, $1); }

params	:				{ $$ = NULL; }
	| params param			{ $$ = treeconsend($1, $2); }

words	:				{ $$ = NULL; }
	| words word			{ $$ = treeconsend($1, $2); }

nlwords :				{ $$ = NULL; }
	| nlwords word			{ $$ = treeconsend($1, $2); }
	| nlwords NL			{ $$ = $1; }

nl	:
	| nl NL

caret 	:	%prec '^'
	| '^'

binder	: LOCAL		{ $$ = nLocal; }
	| LET		{ $$ = nLet; }
	| FOR		{ $$ = nFor; }
	| CLOSURE	{ $$ = nClosure; }

keyword	: '!'		{ $$ = "!"; }
	| '~'		{ $$ = "~"; }
	| '='		{ $$ = "="; }
	| EXTRACT	{ $$ = "~~"; }
	| LOCAL 	{ $$ = "local"; }
	| LET		{ $$ = "let"; }
	| FOR		{ $$ = "for"; }
	| FN		{ $$ = "fn"; }
	| CLOSURE	{ $$ = "%closure"; }
	| MATCH		{ $$ = "match"; }
	| LT		{ $$ = "<"; }    /* Less than operator */
	| GT		{ $$ = ">"; }    /* Greater than operator */
	| LE		{ $$ = "<="; }   /* Less than or equal operator */
	| GE		{ $$ = ">="; }   /* Greater than or equal operator */
	| EQ		{ $$ = "=="; }   /* Equal operator */
	| NE		{ $$ = "!="; }   /* Not equal operator */

