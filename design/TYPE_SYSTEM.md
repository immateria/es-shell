# ES Shell Type System Enhancement

This document describes the enhanced type system added to ES shell that provides better control over numeric types and operations.

## Overview

ES shell now includes:
- **Type conversion primitives** for explicit type casting
- **Type checking primitives** to determine value types  
- **Typed arithmetic primitives** for integer-only operations
- **Mixed-type operations** that handle both integers and floats

## Type Conversion Primitives

### `$&toint value`
Converts a value to an integer.
```sh
echo <={$&toint 3.7}      # Returns: 3
echo <={$&toint 42.9}     # Returns: 42  
echo <={$&toint -1.5}     # Returns: -1
```

### `$&tofloat value`  
Converts a value to a floating-point number.
```sh
echo <={$&tofloat 42}     # Returns: 42
echo <={$&tofloat 3}      # Returns: 3
```

## Type Checking Primitives

### `$&isint value`
Returns 0 (true) if the value is a valid integer, 1 (false) otherwise.
```sh
echo <={$&isint 42}       # Returns: 0 (true)
echo <={$&isint 3.14}     # Returns: 1 (false)
echo <={$&isint abc}      # Returns: 1 (false)
```

### `$&isfloat value`
Returns 0 (true) if the value is a valid floating-point number (contains decimal point), 1 (false) otherwise.
```sh
echo <={$&isfloat 3.14}   # Returns: 0 (true)
echo <={$&isfloat 42}     # Returns: 1 (false, no decimal point)
echo <={$&isfloat abc}    # Returns: 1 (false)
```

## Integer-Only Arithmetic Primitives

These primitives only accept integer values and reject floats.

### `$&intaddition num1 [num2 ...]`
Integer-only addition.
```sh
echo <={$&intaddition 5 7 3}     # Returns: 15
echo <={$&intaddition 5 3.5}     # Error: arguments must be integers
```

### `$&intsubtraction num1 num2 [...]`
Integer-only subtraction.
```sh  
echo <={$&intsubtraction 10 3 2} # Returns: 5
```

### `$&intmultiplication num1 [num2 ...]`
Integer-only multiplication.
```sh
echo <={$&intmultiplication 4 3}  # Returns: 12
```

### `$&intdivision dividend divisor [...]`
Integer-only division (truncates result).
```sh
echo <={$&intdivision 15 4}      # Returns: 3 (integer division)
echo <={$&division 15 4}         # Returns: 3.75 (floating-point division)
```

## Mixed-Type Operations (Existing)

The original arithmetic primitives continue to work with both integers and floats:

- `$&addition` - Handles both integers and floats
- `$&subtraction` - Handles both integers and floats  
- `$&multiplication` - Handles both integers and floats
- `$&division` - Handles both integers and floats, returns proper floating-point results
- `$&modulo` - Handles both integers and floats
- `$&pow` - Handles both integers and floats

## Bitwise Operations (Integer-Only)

Bitwise operations have always required integers and will reject floats:
```sh
echo <={$&bitwiseshiftleft 4 2}   # Returns: 16
echo <={$&bitwiseshiftleft 4.5 2} # Error: value argument must be an integer
```

## Use Cases

### Safe Integer Operations
When you need guaranteed integer arithmetic without floating-point precision issues:
```sh
# Safe integer counter
count = <={$&intaddition $count 1}

# Safe array indexing  
index = <={$&intdivision $total $pagesize}
```

### Type Validation
Before performing operations that require specific types:
```sh
if {$&isint $value} {
    result = <={$&bitwiseshiftleft $value 2}
} {
    value = <={$&toint $value}
    result = <={$&bitwiseshiftleft $value 2}
}
```

### Explicit Type Conversion
When working with mixed numeric data:
```sh  
# Convert string inputs to proper types
intvalue = <={$&toint $input}
floatvalue = <={$&tofloat $input}
```

## Error Handling

Type-specific primitives will fail with clear error messages:
- `$&intaddition 5 3.5` → "arguments must be integers"
- `$&toint abc` → "argument must be a number"
- `$&bitwiseshiftleft 4.5 2` → "value argument must be an integer"

## Migration Notes

- Existing code continues to work unchanged
- The original `%division` now returns proper floating-point results (previously truncated)
- New type-specific primitives are opt-in for stricter type control