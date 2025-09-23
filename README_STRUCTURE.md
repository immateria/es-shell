# es-shell Directory Structure

## Overview

The es-shell codebase has been organized into a logical directory structure to improve maintainability, navigation, and development workflow. This structure groups related functionality together while maintaining all existing functionality.

## Directory Structure

```
es-shell/
├── include/               # Header files
│   ├── config.h          # Build configuration
│   ├── es.h              # Main shell header
│   ├── gc.h              # Garbage collector
│   ├── input.h           # Input handling
│   ├── prim.h            # Primitives system
│   ├── print.h           # Output formatting
│   ├── sigmsgs.h         # Signal messages
│   ├── stdenv.h          # Standard environment
│   ├── syntax.h          # Parser definitions
│   └── debug.h           # Debug utilities
│
├── src/                   # Source code
│   ├── core/             # Core shell engine
│   │   ├── main.c        # Entry point
│   │   ├── eval.c        # Expression evaluator
│   │   └── gc.c          # Garbage collector
│   │
│   ├── primitives/       # Primitive functions
│   │   ├── prim.c        # Primitive system core
│   │   ├── prim-ctl.c    # Control flow primitives
│   │   ├── prim-etc.c    # Miscellaneous primitives
│   │   ├── prim-io.c     # I/O primitives
│   │   ├── prim-math.c   # Mathematical primitives
│   │   └── prim-sys.c    # System interaction primitives
│   │
│   ├── parser/           # Language parsing
│   │   ├── parse.y       # Yacc grammar
│   │   ├── syntax.c      # AST manipulation
│   │   └── token.c       # Tokenizer
│   │
│   ├── io/               # Input/Output handling
│   │   ├── input.c       # Input management
│   │   ├── heredoc.c     # Heredoc processing
│   │   ├── fd.c          # File descriptor handling
│   │   ├── open.c        # File operations
│   │   ├── proc.c        # Process management
│   │   └── term.c        # Terminal handling
│   │
│   ├── data/             # Data structures
│   │   ├── str.c         # String manipulation
│   │   ├── list.c        # List operations
│   │   ├── split.c       # String splitting
│   │   ├── glom.c        # Word globbing
│   │   ├── conv.c        # Type conversions
│   │   ├── dict.c        # Dictionary/map structure
│   │   ├── tree.c        # Tree operations
│   │   └── vec.c         # Vector operations
│   │
│   ├── utils/            # Utility functions
│   │   ├── debug.c       # Debugging utilities
│   │   ├── print.c       # Pretty printing
│   │   ├── match.c       # Pattern matching
│   │   ├── glob.c        # File globbing
│   │   ├── util.c        # General utilities
│   │   └── var.c         # Variable handling
│   │
│   ├── system/           # System integration
│   │   ├── access.c      # File access checks
│   │   ├── signal.c      # Signal handling
│   │   ├── status.c      # Status management
│   │   ├── history.c     # Command history
│   │   ├── opt.c         # Option processing
│   │   ├── except.c      # Exception handling
│   │   └── closure.c     # Closure implementation
│   │
│   ├── build/            # Build-time utilities
│   │   ├── dump.c        # State dumper (for esdump)
│   │   └── version.c     # Version information
│   │
│   ├── stubs/            # Stub implementations
│   │   ├── esdump_stubs.c
│   │   └── eval_stubs.c
│   │
│   ├── tests/            # Test utilities
│   │   ├── simple_test.c
│   │   └── test_initial.c
│   │
│   └── unused/           # Legacy/unused code
│       ├── eval_wrong.c
│       └── glom_original.c
│
├── test/                  # Test suite
├── doc/                   # Documentation
├── examples/              # Example scripts
└── share/                 # Shared resources
```

## Benefits of This Structure

### 1. **Logical Grouping**
- Related functionality is grouped together
- Easy to find specific components
- Clear separation of concerns

### 2. **Improved Navigation**
- IDE/editor navigation is more efficient
- Easier to understand codebase architecture
- Faster development workflow

### 3. **Build System Organization**
- Clean separation between source and headers
- Organized object file placement
- Better dependency management

### 4. **Future Development**
- Easy to add new primitives in `src/primitives/`
- Parser changes contained in `src/parser/`
- System-specific code isolated in `src/system/`

## Key Components by Directory

### `src/core/`
The heart of the shell - evaluation engine, garbage collection, and main entry point.

### `src/primitives/`
All primitive functions that provide the shell's basic operations. This is where you'd add new built-in commands.

### `src/parser/`
Language parsing and syntax tree management. Contains the Yacc grammar and tokenizer.

### `src/io/`
All input/output operations, including file handling, heredocs, and terminal interaction.

### `src/data/`
Data structure implementations - strings, lists, trees, and type conversions.

### `src/utils/`
Utility functions used throughout the shell - debugging, printing, pattern matching.

### `src/system/`
System integration code - signals, process management, command history.

## Building

The build system has been updated to work with this new structure:

```bash
./configure
make clean
make test
```

All existing build commands continue to work exactly as before.

## Migration Notes

- All functionality remains exactly the same
- No changes to shell behavior or syntax
- Test results are identical to pre-reorganization
- Build performance is unchanged
- Header includes updated automatically

## Future Development

When adding new functionality:

1. **New primitives**: Add to `src/primitives/prim-*.c`
2. **Parser changes**: Modify `src/parser/parse.y` or `src/parser/syntax.c`
3. **I/O features**: Add to appropriate file in `src/io/`
4. **Data structures**: Add to `src/data/`
5. **Utilities**: Add to `src/utils/`

This structure provides a solid foundation for the upcoming operator redesign and other enhancements while maintaining the shell's reliability and performance.