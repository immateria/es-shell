# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Overview

es-shell is an extensible shell derived from Plan 9's rc, influenced by functional languages like Scheme and Tcl. It's built around a two-stage compilation process and features a powerful primitive system that bridges C implementations with shell functions.

## Build Instructions

### Prerequisites

**macOS:**
```bash
brew install autoconf automake libtool bison flex pkg-config
```

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y build-essential autoconf automake libtool bison flex pkg-config
```

### Automated Build (Recommended)

```bash
# Build with automated dependency installation and testing
./build.sh

# Build statically linked binary
./build.sh --static

# Build to specific output directory
./build.sh --output-dir ./bin
```

### Manual Build

**Standard build (with existing configure):**
```bash
./configure
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
make test
```

**Build from fresh source checkout:**
```bash
# Generate configure script and build system
libtoolize -qi
autoreconf -fi

# Configure for development with debug symbols
./configure --enable-strict CFLAGS="-O0 -g"
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
make test
```

**Debug build with sanitizers:**
```bash
./configure --enable-strict CFLAGS="-O0 -g -fsanitize=address,undefined" LDFLAGS="-fsanitize=address,undefined"
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
```

### Clean Build States

```bash
make clean          # Remove object files
make distclean       # Remove configure results too
make testclean       # Remove test artifacts
git clean -xdf       # Nuclear option - removes all untracked files
```

## Testing

### Running Tests

```bash
# Run all tests
make test

# Run test harness directly  
./es -ps < test/test.es test/tests/*

# Run specific test file
./es -ps < test/test.es test/tests/math.es

# Run with JUnit XML output (for CI)
./es -ps < test/test.es --junit test/tests/*
```

### Test Structure

Tests use an `assert`-based system in `test/test.es`. Example test pattern:

```bash
test 'arithmetic operations' {
    assert {~ <={$&addition 2 3} 5} 'addition works'
    assert {~ <={$&subtraction 10 4} 6} 'subtraction works'
    
    # Test with descriptive failure message
    assert {~ <={$&multiplication 3 4} 12} 'multiplication produces correct result'
}
```

## Architecture

### Two-Stage Build Process

1. **Stage 1:** `esdump` utility reads `initial.es` and generates `initial.c` containing serialized shell state
2. **Stage 2:** `initial.c` is compiled with core sources to produce the `es` binary

### Core Components

- **parse.y** - Yacc grammar defining shell syntax and building abstract syntax trees
- **eval.c** - Expression evaluator and command dispatcher  
- **gc.c** - Copying garbage collector managing shell memory
- **prim-*.c** - Primitive implementations (arithmetic, I/O, control flow, etc.)
- **initial.es** - Bootstrap script defining initial shell environment

### Primitive System

es-shell primitives follow the pattern: `$&name` in shell code maps to `prim_name()` functions in C.

Function wrappers are defined as: `fn-name = $&primitive`

Example:
```bash
# In initial.es
fn-echo = $&echo          # Shell function wraps C primitive
fn-addition = $&addition  # Arithmetic primitive wrapper
```

## Enhanced Type System

es-shell includes sophisticated type conversion and checking primitives:

### Type Conversion
- `$&toint value` - Convert to integer (truncates floats)
- `$&tofloat value` - Convert to floating-point number

### Type Checking  
- `$&isint value` - Returns 0 (true) if value is integer
- `$&isfloat value` - Returns 0 (true) if value contains decimal point

### Strict Integer Operations
- `$&intaddition num1 [num2 ...]` - Integer-only addition
- `$&intsubtraction num1 num2 [...]` - Integer-only subtraction  
- `$&intmultiplication num1 [num2 ...]` - Integer-only multiplication
- `$&intdivision dividend divisor [...]` - Integer-only division

## Known Issues

### Critical: Bootstrap Build Issues

**Problem:** The `esdump` step fails with "null variable name" during initial.c generation
```bash
./esdump < ./initial.es > initial.c
# Fails with "null variable name"
```

**Root Cause:** Bootstrap dependency issues where functions in `initial.es` try to use other functions defined later in the same file (e.g., using `%count` before it's defined).

**Status:** Major fixes applied:
1. **Function syntax modernization**: Converted ~20 functions from old `fn name params {` to modern `fn-name = @ params {` syntax
2. **Bootstrap dependency resolution**: Replaced problematic `%count` calls with `$#variable` to avoid undefined function calls
3. **Function call evaluation**: Added missing `<={}` wrappers around function calls in conditionals (e.g., `if <={ $predicate $item }`)
4. **Partial success**: Can now build with first 964 lines of `initial.es` using:
   ```bash
   head -964 initial.es | ./esdump > initial.c && make
   ```
5. **Path initialization**: Added basic PATH setup for external command resolution:
   ```bash
   path = (/usr/bin /bin /usr/local/bin)
   ```
6. **Math operators**: Added infix math operators (`plus`, `minus`, `times`, `div`) for convenience

**Current Issues Identified**:
- **I/O System**: Basic `echo` command produces no output, suggesting core I/O issues
- **External Commands**: PATH resolution still not working despite path initialization 
- **Math Primitives**: `$&addition` etc. fail with "bad conversion character in printfmt: %g"
- **Command Execution**: External commands like `ls` return "No such file or directory"

### Critical: Arithmetic Segfaults

**Problem:** Symbolic arithmetic operators cause segmentation faults
```bash
# These crash:
echo <={3 + 5}    # Segfault
echo <={3 - 5}    # Segfault  
echo <={3 * 5}    # Segfault
```

**Workarounds:**
```bash
# Use word-based operators:
echo <={3 plus 5}     # Works (8)
echo <={3 minus 5}    # Works (-2)

# Or use primitives directly:
echo <={$&addition 3 5}        # Works (8)  
echo <={$&subtraction 3 5}     # Works (-2)
echo <={$&multiplication 3 5}  # Works (15)
```

**Root Cause:** Bug in symbolic infix operator parsing/evaluation in `parse.y` or evaluator dispatch.

## Development Workflow

### Fast Iteration
1. Configure once with debug flags: `./configure --enable-strict CFLAGS="-O0 -g"`  
2. Use `make` for incremental builds
3. Use `make V=1` for verbose output when debugging build issues

### Debugging Crashes
```bash
# Debug arithmetic segfaults
gdb --args ./es -c 'echo <={3 + 5}'
# (gdb) run
# (gdb) bt    # Get backtrace on crash
```

### File Modifications
- **Never edit** `initial.c` - it's generated from `initial.es`
- After changing `parse.y`, run `autoreconf -fi && make clean && make`
- After changing `initial.es`, just run `make` (esdump regenerates initial.c)

## Key Files

- **build.sh** - Automated build script with dependency installation
- **configure.ac** - Autotools configuration with `--enable-strict` flag  
- **Makefile.in** - Build rules and targets
- **initial.es** - Shell environment bootstrap (700+ lines defining core functions)
- **esdump.c** / **dump.c** - Utility for serializing shell state to C code
- **prim-math.c** - Arithmetic and type conversion primitives
- **test/test.es** - Test harness with assert-based testing framework
- **PRIORITIES.md** - Development priorities and known issues
- **TYPE_SYSTEM.md** - Detailed type system documentation

## Zsh Coding Conventions

For shell scripts in this repository:

### Functions
```bash
function build-assets
{   emulate -L zsh
    setopt localoptions pipefail

    local -i count
    local -a files  
    local target mode
          count=0
          target=$1
          mode=$2
          files=("${@:3}")

    # Implementation with 4-space indentation
}
```

### Key Rules
- Use `function` keyword, no parentheses
- **kebab-case** for public functions, **_leading-underscore** for private
- Start with `emulate -L zsh; setopt localoptions [options]`  
- Declare locals first: `local -i` (integers), `local -a` (arrays), `local -A` (assoc arrays)
- Modified Allman style: opening brace on next line, body 3 spaces right
- Always quote expansions: `"$var"`
- Use `${(q)var}` for shell-safe quoting
- Use `.N` glob qualifier: `files=(*.tmp(.N))` to avoid "no matches found"
- Prefer zsh builtins: `print`/`printf`, `[[ ]]`, `$((...))`, parameter expansion
- Use `command` prefix to bypass aliases

## Troubleshooting

### Build Issues
- **Missing yacc/bison:** Install with package manager
- **"No rule to make target":** Run `autoreconf -fi` to regenerate build files
- **Linker errors:** Check that readline development packages are installed
- **Build hangs on esdump step:** Use partial build workaround above
- **"null variable name" error:** Use partial build with `head -854 initial.es`

### Runtime Issues  
- **Symbolic arithmetic crashes:** Use workarounds above until fixed
- **Function not found:** Check if primitive exists with `echo <={$&primitives}`
- **Memory issues:** Build with sanitizers for detailed debugging
- **Tests fail:** Expected with partial build - some test functions missing

### Parser Changes
- After modifying `parse.y`: `autoreconf -fi && make clean && make`
- Test grammar changes thoroughly as they affect all shell operations

<citations>
<document>
    <document_type>RULE</document_type>
    <document_id>ZV3a1GnT0fyjpAT0Ywkmmo</document_id>
</document>
</citations>