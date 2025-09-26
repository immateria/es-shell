# hesh — Higher-order Extensible SHell

Based on Byron Rakitzis's `rc` and Paul Haahr's `es`, but rewritten for the modern shell hacker.  
Public domain, no restrictions.  

> hesh wants poppers. you want a modern rc.

## Overview

Hesh shell provides a powerful, functional programming environment with comprehensive I/O redirection, mathematical operations, and extensible architecture. Built for developers who want the elegance of Plan 9 shells with modern programming language features.


## Building and Installation

```bash
# Clone the repository
git clone https://github.com/immateria/es-shell.git
cd es-shell

# Build (requires gcc, bison, readline)
make clean && make

# Run the shell
./es

# Run tests
./es test/tests/redirection.es
```

## Redirection Operators

hesh implements a complete, orthogonal redirection system with 10 operators:

### Basic Redirection
```es
echo "hello" -> output.txt           # Write to file
cat <- input.txt                     # Read from file  
echo "append" ->> log.txt            # Append to file
```

### Advanced Redirection  
```es
cat <-> file.txt                     # Bidirectional read-write
echo "data" <->> existing.txt        # Open-append (create if needed)
echo "new" ->-< file.txt             # Open-create with read capability
echo "more" ->>-< log.txt            # Append with bidirectional access
```

### Here Documents and Strings
```es
# Heredoc (multiline input)
cat <--< MARKER
This is a heredoc
Multiple lines supported
MARKER

# Herestring (single line literal input)  
cat <~ 'This is a herestring literal'
wc -w <~ 'Count words in this text'
```

### Numbered File Descriptors
```es
echo "to stdout" ->[1] /dev/stdout   # Explicit stdout redirection
echo "to stderr" ->[2] /dev/stderr   # Explicit stderr redirection  
echo "to fd 3" ->[3] custom.log      # Custom file descriptor
```

## Mathematical Operations

hesh supports both **prefix** (functional) and **infix** (natural) mathematical syntax:

### Arithmetic Operations
```es
# Prefix notation (functional style)
result = ${+ 5 3}                    # Addition: 8
result = ${- 10 4}                   # Subtraction: 6
result = ${* 7 6}                    # Multiplication: 42
result = ${/ 15 3}                   # Division: 5
result = ${% 17 5}                   # Modulo: 2
result = ${** 2 8}                   # Exponentiation: 256

# Infix notation (natural style)
result = ${5 + 3}                    # Addition: 8
result = ${10 - 4}                   # Subtraction: 6  
result = ${7 * 6}                    # Multiplication: 42
result = ${15 / 3}                   # Division: 5
result = ${17 % 5}                   # Modulo: 2
result = ${2 ** 8}                   # Exponentiation: 256

# Alternative infix operators
result = ${7 × 6}                    # Multiplication with ×
result = ${15 ÷ 3}                   # Division with ÷
result = ${x plus y}                 # Word-based operators
result = ${a times b}                # Natural language style
```

### Bitwise Operations
```es
# Prefix notation
result = ${~and 12 5}                # Bitwise AND: 4
result = ${~or 12 5}                 # Bitwise OR: 13  
result = ${~xor 12 5}                # Bitwise XOR: 9
result = ${~shl 3 2}                 # Left shift: 12
result = ${~shr 12 2}                # Right shift: 3
result = ${~not 5}                   # Bitwise NOT

# Infix notation
result = ${12 bitwise-and 5}         # Bitwise AND: 4
result = ${12 bitwise-or 5}          # Bitwise OR: 13
result = ${12 bitwise-xor 5}         # Bitwise XOR: 9  
result = ${3 shift-left 2}           # Left shift: 12
result = ${12 shift-right 2}         # Right shift: 3
result = ${bitwise-not 5}            # Bitwise NOT (unary)
```

### Comparison Operations  
```es
# Function style
if {greater 10 5} {echo "10 > 5"}    # Numeric comparisons
if {less-equal 3 7} {echo "3 <= 7"}
if {equal 42 42} {echo "Equal"}
if {not-equal 1 2} {echo "Not equal"}

# Infix style (within expressions)
result = ${10 > 5}                   # Returns 0 (true) or 1 (false)
result = ${3 <= 7}                   # Less than or equal
result = ${42 == 42}                 # Equality test
result = ${1 != 2}                   # Not equal test

# Alternative comparison operators
result = ${x greater y}              # Word-based comparisons
result = ${a less-than-or-equal b}   # Verbose style
result = ${value ≥ threshold}        # Unicode operators
result = ${count ≤ limit}            # Mathematical symbols
```

### Advanced Mathematical Functions
```es
# Min/max operations
result = ${min 5 3 8 1}              # Minimum: 1
result = ${max 5 3 8 1}              # Maximum: 8
result = ${5 min 3}                  # Infix min: 3
result = ${8 max 1}                  # Infix max: 8

# Absolute value and utilities
result = ${abs -42}                  # Absolute value: 42
result = ${count (a b c d)}          # Count elements: 4
```

## Dictionary System

```es
# Create dictionaries
person = ${dict-create name Alice age 30 city Boston}

# Access values  
name = ${dict-get name $person}
echo $name                           # Output: Alice

# Check membership
if {dict-contains age $person} {
    echo "Age is defined"
}

# Iterate over entries
dict-foreach $person @ key value {
    echo $key ': ' $value
}
```

## Control Structures

### Enhanced Conditionals
```es
# Traditional if-then-else
if {greater $score 90} {
    echo "Grade A"
} {greater $score 80} {
    echo "Grade B"  
} {
    echo "Grade C"
}

# Flat conditional chains (no nesting!)
cond {greater $score 90} then {echo "A"} 
     elseif {greater $score 80} then {echo "B"}
     else {echo "C"}
```

### Functional Programming
```es
# Higher-order functions
numbers = (1 2 3 4 5)
squares = ${map @ x {* $x $x} $numbers}    # (1 4 9 16 25)
evens = ${filter @ x {~ ${% $x 2} 0} $numbers}  # (2 4)

# Pattern matching  
for (file = *.es) {
    if {~ $file test_*} {
        echo "Test file: $file"
    }
}
```

## Architecture

hesh features a modular, extensible architecture:

```
src/
├── core/eval/          # Modular evaluation engine
│   ├── arithmetic.c    # Mathematical operations  
│   ├── binding.c       # Variable binding and scoping
│   └── control.c       # Control flow structures
├── parser/             # Parsing and tokenization
│   ├── parse.y         # Grammar definition
│   ├── token-redir.c   # Redirection operator parsing
│   └── token-string.c  # String and literal parsing
├── primitives/         # Built-in functions
│   ├── prim-math.c     # Mathematical primitives
│   └── prim-dict.c     # Dictionary operations
└── data/               # Data structures
    ├── dict.c          # Hash table implementation
    └── tree.c          # Abstract syntax trees
```

## Examples

### File Processing Pipeline
```es
# Process log files with redirection and functions
for (logfile = *.log) {
    # Count errors, write summary to report  
    error_count = `{grep -c ERROR <- $logfile}
    echo $logfile ': ' $error_count ' errors' ->> error_report.txt
    
    # Extract error lines to separate file
    grep ERROR <- $logfile -> errors_${logfile}
}
```

### Mathematical Computation
```es
# Calculate factorial using recursion
fn-factorial = @ n {
    if {less-equal $n 1} {
        result 1
    } {
        prev = ${factorial ${- $n 1}}
        result ${* $n $prev}
    }
}

result = ${factorial 5}              # 120
```

### Dictionary-based Configuration
```es
# Configuration management
config = ${dict-create 
    host localhost
    port 8080  
    debug true
    max_connections 100
}

# Use configuration values
if {dict-get debug $config} {
    echo "Debug mode enabled"  
    echo "Connecting to" ${dict-get host $config} ":" ${dict-get port $config}
}
```

## Future Direction

This codebase is transitioning from **es** to **hesh** (Higher-order Extensible SHell), embracing its enhanced capabilities and distinctive character. The name pays homage to Hesh from Sealab 2021 — the reactor operator known for his third-person speech, attention-seeking performances, and memorable catchphrases. Like its namesake, this shell works with core systems, has its own distinctive voice, and delivers when called upon.

**Roadmap:**
- **Enhanced Pattern Matching** — More sophisticated matching capabilities
- **Module System** — Import/export functionality for code organization  
- **Async/Parallel Operations** — Built-in concurrency primitives
- **Network Primitives** — First-class network I/O support
- **Package Management** — Integrated library and tool management

## License

Public domain. No restrictions. Use it, modify it, distribute it freely.

---
> hesh wants poppers. you want a modern rc.
