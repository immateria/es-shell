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
