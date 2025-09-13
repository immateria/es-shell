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
- `make test` reports "Nothing to be done" indicating no test targets.
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

## Runtime Scripts

### [2025-09-13] Runtime Script – share/status.es

**Discovery:** Exposes `$status` in interactive loop

**Details:**
Wraps `%interactive-loop` and `%dispatch` to capture return statuses of commands executed at the REPL, storing them in a local `status` variable.【F:share/status.es†L16-L32】

### [2025-09-13] Runtime Script – share/autoload.es

**Discovery:** Autoloads functions on demand

**Details:**
Overrides `%pathsearch` to look in an XDG-compliant autoload directory, source function definitions, and return the loaded shell function.【F:share/autoload.es†L19-L35】

## Example Scripts

### [2025-09-13] Example – examples/99bottles.es

**Discovery:** Demonstrates loops and closures

**Details:**
Defines helper functions `fn-ne` and `fn-bb` inside a `let` block and uses `forever` to sing "99 Bottles of Beer."【F:examples/99bottles.es†L6-L45】

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

## Test Suite

### [2025-09-13] Test Suite – test/test.es

**Discovery:** Shell-based test harness with JUnit support

**Details:**
Tracks test cases and results, provides an `xml-escape` helper, and emits JUnit-compatible XML in the `report` function when `--junit` is set.【F:test/test.es†L1-L75】

### [2025-09-13] Test Suite – test/testrun.c

**Discovery:** Companion C utility for tests

**Details:**
Implements commands to print a NUL-containing string, sleep, or echo its own name, enabling tests of shell I/O and process behavior.【F:test/testrun.c†L5-L35】
