# Discoveries

## Repository overview
- Es is an extensible shell derived from Plan 9's rc and influenced by functional languages like Scheme and Tcl. The implementation derives from Byron Rakitzis's public domain version of rc.
- `initial.es` holds the initial memory state; `esdebug` is a basic debugger. The project is public domain with maintenance by Soren Dayton and James Haggerty.
- Top-level directories include `doc` (manuals and papers), `examples`, `share` (script library), and `test`.

## Change history (`CHANGES`)
- **0.9.1 → 0.9.2**: readline loads history on startup, minor fixes, and command substitution flatten support.
- **0.9 → 0.9.1**: semver adoption, bug fixes for file handles and environment processing; outstanding signal-handling issues (#7).
- **0.9beta1 → 0.9**: numerous patches including working Ctrl-C and reorganised files; backlog of other patches noted.
- Earlier sections describe migration to GNU autoconf, improved portability, new pattern extraction operator `~~`, and various bug fixes.

## Installation notes (`INSTALL`)
- Standard build: `./configure` then `make`. From source repo, generate configure with `libtoolize -qi` and `autoreconf`.
- Requires an ANSI compiler and POSIX.1-2001 system. Supports build outside source directory and custom install paths.
- Optional GNU readline via `--with-readline` (enabled by default if available).
- OpenBSD build instructions include setting specific `AUTOMAKE_VERSION` and `AUTOCONF_VERSION`, and linking against updated readline.

## Testing
- Without a generated `Makefile`, running `make test` prints "Nothing to be done"; `Makefile.in` later defines a `test` target that executes `test/test.es` with `./es` and a helper `testrun` program.
## Build System

### [2025-09-13] Build System – Makefile.in

**Discovery:** Central build rules and generated files

**Details:**
Defines compilation for numerous C sources into the `es` binary and an `esdump` utility. Targets regenerate parser (`y.tab.c`, `token.h`) and signal message files, and a `test` target runs scripts via `./es`.

### [2025-09-13] Build System – configure.ac

**Discovery:** Autoconf configuration with optional strict mode

**Details:**
Uses autoconf 2.64 to generate `configure`, supporting `--enable-strict` to add pedantic compilation flags. Detects features such as `/dev/fd`, kernel `#!?` support, and optional GNU readline linkage.

## Core Modules

### [2025-09-13] Core Module – main.c

**Discovery:** Entry point handling initialization and CLI options

**Details:**
Initializes garbage collection and conversion routines, sets `$path` and `$pid`, processes flags like `-c`, `-s`, and `-i`, and loads user profiles from `~/.esrc` with exception handling.

### [2025-09-13] Core Module – parse.y

**Discovery:** Yacc grammar defining shell syntax

**Details:**
Specifies tokens for shell constructs (`WORD`, `MATCH`, `FOR`, etc.) and production rules for commands, pipelines, and function definitions, building abstract syntax trees via helper functions such as `mkseq` and `redirappend`.
### [2025-09-13] Core Module – eval.c

**Discovery:** Process invocation and failure hooks

**Details:**
`failexec` looks for a `fn-%exec-failure` handler before printing errors, and `forkexec` builds an environment vector, forks, and executes commands, returning status as a list.【F:eval.c†L7-L45】

### [2025-09-13] Core Module – gc.c

**Discovery:** Copying garbage collector with space management

**Details:**
Defines `Space` structures and macros to track used and free bytes, maintaining global roots and separate new/old spaces for collection.【F:gc.c†L8-L49】

### [2025-09-13] Core Module – prim.c

**Discovery:** Primitive dispatch table

**Details:**
Looks up primitives in a dictionary and initializes the table by aggregating modules like controlflow and io.【F:prim.c†L8-L49】

### [2025-09-13] Core Module – prim-io.c

**Discovery:** Redirection helper with deferred file descriptor moves

**Details:**
The `redir` function parses destinations and sources, defers close or dup operations, evaluates the command, and ensures cleanup even on exceptions.【F:prim-io.c†L20-L38】

### [2025-09-13] Core Module – signal.c

**Discovery:** Signal bookkeeping and mapping

**Details:**
Maintains arrays tracking caught signals and effects, provides `signumber` to resolve names like `sigINT` to numeric codes, and supports interruption of blocking calls.【F:signal.c†L10-L52】

### [2025-09-13] Core Module – list.c

**Discovery:** GC-aware list allocation

**Details:**
`DefineTag` registers list nodes with the garbage collector, while `mklist` allocates and links terms under GC control.【F:list.c†L11-L18】

### [2025-09-13] Core Module – var.c

**Discovery:** Environment variable storage and vector expansion

**Details:**
`VECPUSH` macro grows vectors as needed, and dictionaries `vars` and `noexport` track shell variables and export exclusions.【F:var.c†L17-L39】

### [2025-09-13] Core Module – syntax.c

**Discovery:** Tree manipulation utilities

**Details:**
Functions like `treecons` and `treeappend` construct and extend abstract syntax tree lists, ensuring correct node kinds.【F:syntax.c†L11-L32】

### [2025-09-13] Core Module – glob.c

**Discovery:** Quoting-aware wildcard expansion

**Details:**
`haswild` detects unquoted wildcard characters, and `dirmatch` uses `opendir`/`readdir` to match directory entries while hiding dot-files unless requested.【F:glob.c†L16-L58】

### [2025-09-13] Core Module – open.c

**Discovery:** File opening wrappers and tty duplication

**Details:** `eopen` maps custom `OpenKind` values to POSIX flags, and `opentty` duplicates `/dev/tty` to a descriptor >=3, preserving errno on failure.【F:open.c†L21-L48】

### [2025-09-13] Core Module – conv.c

**Discovery:** Format handlers for lists, trees, closures, and terms

**Details:** Provides conversions like `%L` for lists, `%T` for trees, `%C` for closures, and `%E` for terms, installing them via `initconv`.【F:conv.c†L7-L118】【F:conv.c†L479-L488】

### [2025-09-13] Core Module – access.c

**Discovery:** Permission and type checks with path suffixing

**Details:** `ingroupset` caches group IDs for access tests, and `$&access` parses flags like `-rwx` and `-fdl` before evaluating each path.【F:access.c†L25-L45】【F:access.c†L110-L138】

### [2025-09-13] Core Module – history.c

**Discovery:** Readline-backed command history

**Details:** Builds input lines in a `histbuffer` and appends them to a configurable history file via `append_history`, reloading on demand.【F:history.c†L42-L65】【F:history.c†L90-L102】

### [2025-09-13] Core Module – input.c

**Discovery:** Input abstraction with pushback and interactive readline

**Details:** `unget` manages a pushback buffer, while `fdfill` reads from file descriptors or wraps readline when interactive on stdin.【F:input.c†L69-L101】【F:input.c†L168-L198】

### [2025-09-13] Core Module – split.c

**Discovery:** Separator-aware string splitting

**Details:** `startsplit` seeds delimiter tables, `stepsplit` consumes characters into buffered terms, and `endsplit` returns the accumulated list.【F:split.c†L14-L105】

### [2025-09-13] Core Module – str.c

**Discovery:** Formatted string builders and `StrList` helpers

**Details:** Provides `str`, `pstr`, and `mprint` to allocate formatted strings in different regions and defines GC-aware string lists.【F:str.c†L7-L131】

### [2025-09-13] Core Module – dump.c

**Discovery:** Generate C source representing runtime state

**Details:** Serializes strings, lists, and trees into static C declarations, ensuring printable output and deduplicating nodes.【F:dump.c†L1-L80】【F:dump.c†L84-L160】

### [2025-09-13] Core Module – match.c

**Discovery:** Pattern and list matching with quoting support

**Details:** `rangematch` parses character classes respecting quotes, and `match` plus `listmatch` handle glob-like wildcards over strings and lists.【F:match.c†L26-L107】【F:match.c†L110-L159】

### [2025-09-13] Core Module – proc.c

**Discovery:** Process tracking and wait primitives

**Details:** `efork` records child processes, `ewait` reaps them with status reporting, and `apids` lists background jobs.【F:proc.c†L22-L63】【F:proc.c†L119-L190】

### [2025-09-13] Core Module – fd.c

**Discovery:** Deferred file descriptor remapping

**Details:** Maintains a stack of `Defer` records so parents delay `dup2` or `close` operations until `closefds` time, and provides mapping utilities.【F:fd.c†L6-L115】

### [2025-09-13] Core Module – heredoc.c

**Discovery:** Here-document parsing and variable interpolation

**Details:** `snarfheredoc` reads until an EOF marker, expanding `$var` tokens when unquoted, while `queueheredoc` schedules pending documents.【F:heredoc.c†L1-L114】【F:heredoc.c†L116-L145】

### [2025-09-13] Core Module – except.c

**Discovery:** Exception stack with `fail` helper

**Details:** `throw` unwinds handlers and restores variable bindings, and `fail` packages errors as `$&parse` exceptions before throwing.【F:except.c†L7-L67】

### [2025-09-13] Core Module – closure.c

**Discovery:** Extract bindings from `%closure` syntax

**Details:** Builds `Closure` objects, reverses binding lists, and `extractbindings` walks nested closures collecting variable definitions.【F:closure.c†L10-L49】【F:closure.c†L56-L147】

### [2025-09-13] Core Module – util.c

**Discovery:** Miscellaneous helpers and safe allocation

**Details:** Wraps `strerror`, implements path utilities like `isabsolute`, and provides checked `ealloc`/`erealloc` memory routines.【F:util.c†L17-L54】【F:util.c†L61-L83】

### [2025-09-13] Core Module – tree.c

**Discovery:** Parse tree creation and GC scanning

**Details:** `gmk` allocates nodes for each `NodeKind`, with `Tree1Scan` and `Tree2Scan` forwarding child pointers during garbage collection.【F:tree.c†L9-L31】【F:tree.c†L75-L114】

### [2025-09-13] Core Module – term.c

**Discovery:** Term wrapping and closure extraction

**Details:** `mkterm` stores either raw strings or closures, and `getclosure` lazily parses strings starting with `{` or `$&` into closures.【F:term.c†L9-L52】

### [2025-09-13] Core Module – glom.c

**Discovery:** Tree-to-list expansion with quoting metadata

**Details:** `concat` and `qconcat` compute Cartesian products of terms and quote flags, while `subscript` selects list ranges via indices.【F:glom.c†L1-L83】【F:glom.c†L86-L149】

### [2025-09-13] Core Module – access.c

**Discovery:** Path-aware permission checks

**Details:** `PRIM(access)` parses options like `-rwx` and tests file types via `testfile`, combining user/group masks for permission evaluation.【F:access.c†L25-L64】【F:access.c†L110-L134】

### [2025-09-13] Core Module – conv.c

**Discovery:** List and tree formatting helpers

**Details:** Implements `%L` to print lists and `%T` to recursively format syntax trees using `treecount` and `binding` utilities.【F:conv.c†L7-L18】【F:conv.c†L48-L92】

### [2025-09-13] Core Module – dict.c

**Discovery:** GC-integrated hash dictionaries

**Details:** Uses Haahr's `strhash2` and `DefineTag` macros to allocate variable-length tables and forward pointers during collection.【F:dict.c†L14-L44】【F:dict.c†L57-L96】

### [2025-09-13] Core Module – dump.c

**Discovery:** Emit C source for runtime state

**Details:** `$&dumpstate` writes constant declarations for strings, lists, and trees, reusing names and disabling GC during traversal.【F:dump.c†L10-L32】【F:dump.c†L45-L76】

### [2025-09-13] Core Module – es.h

**Discovery:** Fundamental structures and node kinds

**Details:** Declares `List`, `Binding`, `Closure`, and enumerates `NodeKind` variants for parsing and evaluation.【F:es.h†L10-L37】【F:es.h†L45-L58】

### [2025-09-13] Core Module – glob.c

**Discovery:** Quoting-aware filename expansion

**Details:** `haswild` detects unquoted metacharacters and `dirmatch` scans directories while hiding dot files by default.【F:glob.c†L11-L35】【F:glob.c†L46-L82】

### [2025-09-13] Core Module – match.c

**Discovery:** Pattern and range matching

**Details:** `rangematch` handles character classes and `match` applies `*`, `?`, and `[]` with quote awareness.【F:match.c†L22-L55】【F:match.c†L58-L107】

### [2025-09-13] Core Module – open.c

**Discovery:** File opening abstraction

**Details:** Maps `OpenKind` enums to `open` flags and duplicates `/dev/tty` descriptors via `opentty`.【F:open.c†L15-L33】【F:open.c†L35-L47】

### [2025-09-13] Core Module – split.c

**Discovery:** IFS-based string splitting

**Details:** `startsplit` caches separators and `stepsplit` accumulates terms, with `fsplit` iterating over list inputs.【F:split.c†L14-L37】【F:split.c†L39-L74】【F:split.c†L107-L120】

### [2025-09-13] Core Module – status.c

**Discovery:** Status list utilities

**Details:** Defines canonical true/false lists and converts shell results to exit codes with `istrue` and `exitstatus`.【F:status.c†L6-L14】【F:status.c†L16-L53】

### [2025-09-13] Core Module – token.c

**Discovery:** Lexical scanner with token tables

**Details:** Arrays `nw` and `dnw` classify characters, and `yylex` builds tokens while tracking prompts and keywords.【F:token.c†L1-L46】【F:token.c†L144-L200】

### [2025-09-13] Core Module – print.c / print.h

**Discovery:** Custom formatted output library

**Details:** `Format` tracks flags like `FMT_zeropad`, and converters such as `sconv` and `intconv` build strings and numbers.【F:print.c†L13-L42】【F:print.c†L49-L64】【F:print.h†L3-L27】

### [2025-09-13] Core Module – syntax.h

**Discovery:** AST construction prototypes

**Details:** Macros `CAR`/`CDR` expose node fields, with declarations for builders like `treecons`, `mkseq`, and `mkpipe`.【F:syntax.h†L3-L37】

### [2025-09-13] Core Module – var.h

**Discovery:** Variable representation flags

**Details:** `Var` holds definition lists and environment strings, with flags `var_hasbindings` and `var_isinternal`.【F:var.h†L1-L13】

### [2025-09-13] Core Module – vec.c

**Discovery:** GC-tracked argument vectors

**Details:** `mkvector` allocates room for argv/envp arrays and `sortvector` orders entries using `qsort`.【F:vec.c†L8-L16】【F:vec.c†L34-L45】【F:vec.c†L56-L59】

### [2025-09-13] Core Module – gc.h

**Discovery:** Garbage collector tagging API

**Details:** `Tag` structures pair copy and scan functions, and buffer helpers like `openbuffer` and `bufputc` manage temporary memory.【F:gc.h†L9-L31】【F:gc.h†L39-L55】

## Runtime Scripts

### [2025-09-13] Runtime Script – share/status.es

**Discovery:** Exposes `$status` in interactive loop

**Details:**
Wraps `%interactive-loop` and `%dispatch` to capture return statuses of commands executed at the REPL, storing them in a local `status` variable.【F:share/status.es†L16-L32】

### [2025-09-13] Runtime Script – share/autoload.es

**Discovery:** Autoloads functions on demand

**Details:**
Overrides `%pathsearch` to look in an XDG-compliant autoload directory, source function definitions, and return the loaded shell function.【F:share/autoload.es†L19-L35】

### [2025-09-13] Runtime Script – share/cdpath.es

**Discovery:** rc-style `cdpath` support

**Details:** Overrides `cd` to search `$cdpath` for relative directories via `%cdpathsearch` and provides setters for `CDPATH`.【F:share/cdpath.es†L1-L24】【F:share/cdpath.es†L30-L56】

### [2025-09-13] Runtime Script – share/interactive-init.es

**Discovery:** Hook for initialization before REPL

**Details:** Wraps `%interactive-loop` to invoke `%interactive-init` under a `catch`, preventing uncaught exceptions from terminating the shell.【F:share/interactive-init.es†L1-L23】

### [2025-09-13] Runtime Script – share/path-cache.es

**Discovery:** Function cache for executables

**Details:** Intercepts `%pathsearch` to memoize program paths, offers `recache` and `precache`, and tracks cached names in `$path-cache`.【F:share/path-cache.es†L1-L30】【F:share/path-cache.es†L32-L71】【F:share/path-cache.es†L73-L95】

## Example Scripts

### [2025-09-13] Example – examples/99bottles.es

**Discovery:** Demonstrates loops and closures

**Details:**
Defines helper functions `fn-ne` and `fn-bb` inside a `let` block and uses `forever` to sing "99 Bottles of Beer."【F:examples/99bottles.es†L6-L45】

### [2025-09-13] Example – examples/adventure.es

**Discovery:** Text adventure-style shell

**Details:** Implements commands like `ask`, `instructions`, and `help` to explore the filesystem as rooms and items. 【F:examples/adventure.es†L23-L63】【F:examples/adventure.es†L71-L96】

### [2025-09-13] Example – examples/cd_colourprompt.es

**Discovery:** Colored prompt on directory changes

**Details:** Redefines `cd` to update `$prompt` with ANSI escape sequences showing hostname and working directory. 【F:examples/cd_colourprompt.es†L1-L5】

### [2025-09-13] Example – examples/cd_follow-symbolic.es

**Discovery:** `cd` that resolves symbolic links

**Details:** Maintains a `$cwd` variable and reconstructs paths component-by-component, ensuring `cd symlink/..` stays within the link. 【F:examples/cd_follow-symbolic.es†L1-L45】

### [2025-09-13] Example – examples/es-mode.el

**Discovery:** Emacs major mode for es scripts

**Details:** Defines `es-mode` with keyword highlighting and syntax table entries for underscores, hyphens, and periods. 【F:examples/es-mode.el†L1-L29】【F:examples/es-mode.el†L32-L40】

## Utilities

### [2025-09-13] Utility – esdebug

**Discovery:** Interactive debugger for es

**Details:** Launches programs through es, exposes commands like `_debug-trace` and `_debug-break`, and communicates via file descriptor 3. 【F:esdebug†L1-L43】【F:esdebug†L63-L97】【F:esdebug†L111-L150】

## Documentation

### [2025-09-13] Documentation – doc/TODO

**Discovery:** Pending improvements

**Details:**
Lists bugs such as lacking closure inheritance in subshells and proposals like tail recursion and `%parse` exposure for macros.【F:doc/TODO†L2-L16】

### [2025-09-13] Documentation – doc/ERRATA

**Discovery:** Corrections to published materials

**Details:**
Notes renaming of the `<` operator to `<=`, enriched error exceptions with routine names, and updated `in` function example.【F:doc/ERRATA†L1-L23】

### [2025-09-13] Documentation – doc/es.1

**Discovery:** Manual page overview

**Details:**
Describes es as an extensible shell combining Unix shell features with functional programming concepts, with syntax derived from rc and customizable semantics.【F:doc/es.1†L182-L207】

### [2025-09-13] Documentation – README.md

**Discovery:** Project overview and provenance

**Details:** Describes es as an extensible shell derived from Plan 9's rc and influenced by functional languages, and points to manuals, examples, and official mirrors. 【F:README.md†L3-L13】【F:README.md†L31-L35】

## Test Suite

### [2025-09-13] Test Suite – test/test.es

**Discovery:** Shell-based test harness with JUnit support

**Details:**
Tracks test cases and results, provides an `xml-escape` helper, and emits JUnit-compatible XML in the `report` function when `--junit` is set.【F:test/test.es†L1-L75】

### [2025-09-13] Test Suite – test/testrun.c

**Discovery:** Companion C utility for tests

**Details:**
Implements commands to print a NUL-containing string, sleep, or echo its own name, enabling tests of shell I/O and process behavior.【F:test/testrun.c†L5-L35】

### [2025-09-13] Test Suite – test/tests/glob.es

**Discovery:** Filesystem globbing scenarios

**Details:** Creates temporary directories to verify `*`, `?`, and range patterns, including edge cases and quoted literals. 【F:test/tests/glob.es†L1-L18】【F:test/tests/glob.es†L21-L44】【F:test/tests/glob.es†L46-L77】

### [2025-09-13] Test Suite – test/tests/match.es

**Discovery:** Matching semantics and error cases

**Details:** Compares `match` with `if` logic over various subjects and ensures unmatched cases raise errors without special `break` handling. 【F:test/tests/match.es†L1-L29】【F:test/tests/match.es†L31-L83】【F:test/tests/match.es†L86-L93】
## Build Troubleshooting

### [2025-09-13] Build – Missing libtoolize

**Discovery:** Autotools tool `libtoolize` was absent, preventing configure generation.

**Details:** Initial attempt to run `libtoolize -qi` failed with "command not found". Installed required packages via `apt-get install -y libtool autoconf`, enabling subsequent build steps.

### [2025-09-13] Build – Build and tests succeed

**Discovery:** Build system runs after dependencies installed

**Details:** Ran `libtoolize -qi`, `autoreconf`, `./configure`, and `make`, producing the `es` binary linked with `readline`. `make test` executed 20 checks without failures, confirming a clean build.

### [2025-09-13] Build – Strict mode

**Discovery:** Build succeeds with strict compiler flags

**Details:** Configured with `--enable-strict`, enabling `-ansi -pedantic` and GC assertion flags. `make` and `make test` completed without errors.

### [2025-09-13] Build – No readline

**Discovery:** Build succeeds without GNU readline

**Details:** Used `--with-readline=no` to disable the dependency; `make` linked without `-lreadline` and `make test` still passed all 20 checks.
### [2025-09-13] Build – Static with readline

**Discovery:** Static linking with GNU readline succeeds with glibc warnings

**Details:** Configured with `LDFLAGS=-static`; `make` emitted warnings about `getpwent`, `setpwent`, and `getpwuid` requiring glibc shared libraries, but `make test` passed all 20 checks.

### [2025-09-13] Build – Static without readline

**Discovery:** Static build works without GNU readline

**Details:** Ran `./configure LDFLAGS=-static --with-readline=no`; build still warned about `getpwnam` needing glibc shared libraries, yet `make test` reported 20/20 passing.
### [2025-09-13] Build – Clang compiler

**Discovery:** Build succeeds with Clang

**Details:** Installed `clang` and missing autotools packages, regenerated build scripts with `libtoolize -qi` and `autoreconf -i`, configured with `CC=clang`, then `make` and `make test` completed with 20/20 passing.
### [2025-09-13] Build – Clang static

**Discovery:** Static linking with Clang works with glibc warnings

**Details:** Configured with `CC=clang` and `LDFLAGS=-static`; the linker warned about `getpwent`, `setpwent`, `endpwent`, `getpwnam`, and `getpwuid` needing glibc shared libraries, but `make test` still reported all checks passing.

### [2025-09-13] Build – TinyCC compiler

**Discovery:** Build and tests succeed with tcc

**Details:** Installed TinyCC and built using `CC=tcc` with default options. Compilation emitted "function might return no value" warnings, yet `make test` completed with the full suite passing.

### [2025-09-13] Build – AddressSanitizer

**Discovery:** GCC build with address sanitizer passes tests

**Details:** Configured with `CC=gcc CFLAGS='-fsanitize=address' LDFLAGS='-fsanitize=address'`; despite "control reaches end of non-void function" warnings, compilation succeeded and `make test` reported all 20 checks passing.

### [2025-09-13] Build – musl static linking

**Discovery:** Static binary using musl-gcc passes tests

**Details:** Installed `musl-tools` and built with `CC=musl-gcc LDFLAGS=-static`, producing a fully static executable that ran the full test suite without failures.

### [2025-09-13] Build – Portable C Compiler

**Discovery:** pcc builds and runs tests with minor warnings

**Details:** Installed `pcc` and compiled using `CC=pcc`. Numerous "unsupported attribute" warnings appeared, but the shell and test suite still executed successfully.

### [2025-09-13] Build – 32-bit GCC attempt

**Discovery:** 32-bit cross-compilation fails during configure

**Details:** Invoked `./configure CC='gcc -m32'`, which stopped with "C compiler cannot create executables," indicating missing 32-bit development libraries.

### [2025-09-13] Build – ARM64 cross-compilation

**Discovery:** Generates ARM64 binary after prebuilding host tools

**Details:**
Installed cross-compilers `gcc-aarch64-linux-gnu` and `gcc-arm-linux-gnueabihf`. To satisfy the build's `esdump` dependency, a native `esdump` was used to pre-generate `initial.c`, then reused while configuring with `es_cv_local_getenv=no CC=aarch64-linux-gnu-gcc --host=aarch64-linux-gnu`. `make es` produced an ARM64 `es` binary, but `make test` failed with "Exec format error" because the host cannot execute AArch64 binaries. `file es` confirmed the output as an AArch64 ELF executable.

### [2025-09-13] Build – Android/Termux target attempt

**Discovery:** Clang Android cross build fails at configure stage

**Details:**
Attempted to mimic a Termux environment with `es_cv_local_getenv=no CC="clang --target=aarch64-linux-android21" ./configure --host=aarch64-linux-android`, but configure aborted with "C compiler cannot create executables," indicating the lack of an Android sysroot or NDK.
