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

## Comprehensive Infix Operator Support

es-shell provides extensive infix operator support with multiple syntax styles for maximum flexibility and readability.

### Arithmetic Operators

All arithmetic operations support three syntax styles:

| Operation | Word-based | Short Form | Symbolic |
|-----------|------------|------------|----------|
| Addition | `plus` | - | `+` |
| Subtraction | `minus`, `subtract` | - | `-` |
| Multiplication | `multiply`, `times` | - | `*` |
| Division | `divide`, `divided-by` | `div` | `/` |
| Modulo | `mod`, `modulo` | - | `%` |
| Power | `power`, `raised-to` | `pow` | `**` |
| Minimum | `minimum` | `min` | - |
| Maximum | `maximum` | `max` | - |

### Comparison Operators

| Operation | Word-based | Short Form | Symbolic/Underscore |
|-----------|------------|------------|--------------------|
| Greater than | `greater`, `greater-than` | `gt` | `>`, `_gt_`, `_>` |
| Less than | `less`, `less-than` | `lt` | `<`, `_lt_`, `_<` |
| Greater/equal | `greater-equal` | `ge`, `gte` | `>=`, `_ge_`, `_>=` |
| Less/equal | `less-equal` | `le`, `lte` | `<=`, `_le_`, `_<=` |
| Equal | `equal`, `equals` | `eq` | `==`, `_eq_`, `_==` |
| Not equal | `not-equal` | `ne`, `neq` | `!=`, `_ne_`, `_!=` |

### Bitwise Operators

| Operation | Word-based | Symbolic |
|-----------|------------|----------|
| Bitwise AND | `bitwiseand` | `∧` |
| Bitwise OR | `bitwiseor` | `∨` |
| Bitwise XOR | `bitwisexor` | `⊻` |
| Bitwise NOT | `bitwisenot` | `~not` |
| Shift Left | `shift-left` | `~shl` |
| Shift Right | `shift-right` | `~shr` |

### Usage Examples

```bash
# Current syntax (still works)
echo <={10 plus 5}        # Word-based: 15
echo <={10 + 5}           # Symbolic: 15  
echo <={10 gt 5}          # Short comparison: 0 (true)
echo <={10 greater 5}     # Word comparison: 0 (true)
echo <={10 _gt_ 5}        # Underscore style: 0 (true)

# NEW modern syntax (when implemented)
echo ${10 + 5}            # Clean expression evaluation: 15
echo ${10 > 5}            # True comparison operators: 0 (true)
echo ${10 ≥ 5}            # Unicode operators: 0 (true)

# Complex expressions with precedence:
echo ${2 + 3 * 4}         # Natural precedence: 14
echo ${(2 + 3) * 4}       # Grouped: 20

# Modern conditional syntax (planned)
cond {$score >= 90} then {echo "A grade"}
cond {$balance > 1000} then {echo "Sufficient funds"}
cond {$value ≈ 3.14} then {echo "Close to pi"}

# Advanced operators (planned)
config ??= load_defaults()        # Nullish coalescing
PATH =+ "/usr/local/bin:"         # Prepend operator
find . -name "*.tmp" |? rm -f     # Conditional pipe
```

**Note:** All operator styles work in arithmetic expressions `<={...}`, conditional tests `cond {...}`, and complex nested expressions.

## Advanced Control Structures

### Flat Conditional Chains with `cond`

es-shell provides a comprehensive `cond` function that eliminates the need for nested conditional statements through flat `if-then-elseif-else` chains.

#### Syntax
```bash
cond {condition} then {action}
cond {condition} then {action} else {action}
cond {condition} then {action} elseif {condition} then {action} else {action}
```

#### Benefits Over Nested `if`

**Old nested approach:**
```bash
if {greater-equal $score 90} {
    echo "A grade"
} {
    if {greater-equal $score 80} {
        echo "B grade"  
    } {
        if {greater-equal $score 70} {
            echo "C grade"
        } {
            echo "F grade"
        }
    }
}
```

**New flat approach:**
```bash
cond {$score greater-equal 90} then {
    echo "A grade"
} elseif {$score greater-equal 80} then {
    echo "B grade"
} else {
    echo "C grade"
}
```

#### Advanced Examples

```bash
# Financial calculation with infix operators
balance = 1000
fee = 50
remaining = <={$balance minus $fee}
cond {$remaining _gt_ 900} then {
    echo "Balance OK after fee"
} else {
    echo "Low balance warning"
}

# Financial decision with two conditions
age = 25
income = 50000
cond {$age _lt_ 18} then {
    echo "Minor - no loan"
} elseif {$income _lt_ 30000} then {
    echo "Insufficient income"
} else {
    echo "Loan approved"
}
```

## Comprehensive Operator Redesign (In Development)

**Status:** Design finalized, implementation roadmap defined.

### Objective
Transform es-shell into a modern, expressive shell by:
1. Freeing up `<`, `>`, `<=`, `>=`, `==`, `!=` for use as comparison operators
2. Implementing intuitive arrow-based redirection syntax
3. Introducing `${...}` for function evaluation (replacing `<={...}`)
4. Adding Unicode operator alternatives for mathematical clarity
5. Enhancing assignment and pipeline operators

### Core Changes

#### Function Evaluation
| Current | New | Description |
|---------|-----|-------------|
| `<={...}` | `${...}` | Function/expression evaluation |

**Rationale:** `${...}` mirrors variable expansion patterns and is familiar to modern developers.

#### Redirection Operators
| Current | New | Description |
|---------|-----|-------------|
| `<` | `<-` | Input from file |
| `>` | `->` | Output to file |
| `>>` | `->>` | Append to file |
| `2>` | `!->` | Stderr to file |
| `&>` | `&->` | Both stdout and stderr |
| `<<EOF` | `<--<EOF` | Heredoc |
| `<<'EOF'` | `<--<'EOF'` | Quoted heredoc (no expansion) |
| `<<<` | `<=` | Here string |
| `<>` | `<->` | Read-write mode |

#### Comparison Operators with Unicode
| ASCII | Unicode | Description |
|-------|---------|-------------|
| `<` | - | Less than |
| `>` | - | Greater than |
| `<=` | `≤` | Less than or equal |
| `>=` | `≥` | Greater than or equal |
| `==` | `≡` | Equal |
| `!=` | `≠` | Not equal |
| `===` | `≣` | Strict equality (type-aware) |
| `≈` | - | Approximately equal |

#### Enhanced Assignment Operators
| Operator | Description | Example |
|----------|-------------|----------|
| `??=` | Assign if null/unset | `config ??= "default"` |
| `\|\|=` | Assign if falsy | `verbose \|\|= false` |
| `=+` | Prepend | `PATH =+ "/usr/local/bin:"` |
| `.=` | String concatenation | `msg .= " done"` |

#### Advanced Pipeline Operators
| Operator | Description | Example |
|----------|-------------|----------|
| `\|?` | Conditional pipe | `find . \|? xargs rm` |
| `\|\|>` | Parallel pipe | `data \|\|> proc1 \|\|> proc2` |
| `=->` | Tee operation | `download =-> file.bin` |

### Migration Strategy

**Phase 1:** Dual syntax support with compatibility mode
**Phase 2:** Transition with automated migration tools
**Phase 3:** Modern syntax as default
**Phase 4:** Optional legacy removal in v2.0

### Benefits
- **Modern & Intuitive:** Aligns with JavaScript, Python, and other contemporary languages
- **Clear Semantics:** Arrow operators visually indicate data flow direction
- **Familiar Syntax:** `${...}` mirrors shell variable expansion patterns
- **Unicode Support:** Mathematical operators improve readability
- **Powerful Features:** Nullish coalescing, prepend, conditional pipes

## Enhanced Testing Infrastructure

### Comprehensive Test Suite

es-shell now includes extensive regression testing for all new features:

- **`test-all-infix.es`**: Complete test of all infix operator styles (word, short, symbolic)
- **`test-cond-elseif.es`**: Basic cond functionality tests
- **`test-cond-infix.es`**: Comprehensive infix operators within cond tests
- **`test/run-math-bitwise-primitives.sh`**: Automated arithmetic and bitwise primitive testing

### Test Coverage

✅ **Arithmetic Operations**: All operator styles and precedence rules  
✅ **Comparison Operations**: Word, short, and symbolic forms  
✅ **Control Structures**: Flat conditional chains with elseif support  
✅ **Complex Expressions**: Nested arithmetic with parentheses  
✅ **Bitwise Operations**: All bitwise primitives and infix forms  
✅ **Error Handling**: Graceful error messages for invalid syntax  

### Running Tests

```bash
# Run comprehensive infix operator test
./test-all-infix.es

# Run cond function tests
./test-cond-elseif.es
./test-cond-infix.es

# Run automated math/bitwise suite
./test/run-math-bitwise-primitives.sh
```

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
- **design/PRIORITIES.md** - Development priorities and known issues
- **design/TYPE_SYSTEM.md** - Detailed type system documentation

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

## Current Status & Known Issues

### ✅ Resolved Issues

- **Arithmetic Operations**: All infix operators (word-based, symbolic, short forms) work correctly
- **Bootstrap Build**: Build system works reliably with full `initial.es`  
- **Control Structures**: Comprehensive `cond` function with elseif chains implemented
- **Type System**: All type conversion and checking primitives functional
- **Testing**: Complete regression test suite passes

### 🚧 Planned Features

- **Operator Redesign**: Complete overhaul with `${...}` evaluation, arrow-based I/O, Unicode operators
- **Enhanced Assignment**: Nullish coalescing (`??=`), prepend (`=+`), logical assignments
- **Advanced Pipelines**: Conditional (`|?`), parallel (`||>`), tee (`=->`) operations
- **Strict Typing**: Type-aware equality (`===`) and comparison operators
- **Pattern Matching**: Built-in regex (`=~`) and glob (`~=`) operators
- **Enhanced Error Messages**: Context-aware, helpful error reporting
- **Performance Optimizations**: Optimized operator dispatch and expression evaluation

### 📋 Migration Tasks

1. Implement Phase 1 of redirection redesign (dual syntax support)
2. Update documentation for new symbolic comparison operators
3. Add comprehensive examples to user manual

## Troubleshooting

### Build Issues
- **Missing yacc/bison:** Install with package manager
- **"No rule to make target":** Run `autoreconf -fi` to regenerate build files
- **Linker errors:** Check that readline development packages are installed

### Parser Changes
- After modifying `parse.y`: `autoreconf -fi && make clean && make`
- Test grammar changes thoroughly as they affect all shell operations

### Operator Migration (When Implemented)
- **Compatibility Mode:** Use `--classic-operators` flag for legacy syntax
- **Deprecation Warnings:** Configurable warnings for old `<`, `>`, `<={...}` syntax
- **Migration Tool:** `es-migrate --convert-operators` for automatic conversion
- **Dual Support:** Both syntaxes work during transition period
- **Unicode Detection:** Automatic fallback to ASCII when UTF-8 unavailable
- **Testing:** Comprehensive test suite for all operator combinations

<citations>
<document>
    <document_type>RULE</document_type>
    <document_id>ZV3a1GnT0fyjpAT0Ywkmmo</document_id>
</document>
</citations>