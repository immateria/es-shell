# ES Shell if-then Control Structure

## Overview

The ES shell now includes a traditional `if-then-else` control structure with full comparison operator support. This provides more readable and familiar syntax for conditional logic.

## Syntax

```bash
if-then condition then action else action
```

## Examples

### Basic Usage

```bash
# Simple comparison
x = 10
if-then {greater $x 5} then {
    echo 'x is greater than 5'
} else {
    echo 'x is not greater than 5'
}
```

### Without Else Clause

```bash
score = 95
if-then {greater-equal $score 90} then {
    echo 'Excellent score!'
}
```

### Nested if-then Statements

```bash
score = 87
if-then {greater-equal $score 90} then {
    echo 'Grade: A'
} else {
    if-then {greater-equal $score 80} then {
        echo 'Grade: B'
    } else {
        echo 'Grade: C or below'
    }
}
```

## Comparison Operators

All comparison operators return exit status (0 = true, 1 = false) and work with the `if-then` structure:

### Numeric Comparisons

- `greater a b` - Returns true if a > b
- `less a b` - Returns true if a < b  
- `greater-equal a b` - Returns true if a >= b
- `less-equal a b` - Returns true if a <= b
- `equal a b` - Returns true if a == b (within epsilon for floating point)
- `not-equal a b` - Returns true if a != b

### Usage Examples

```bash
# All of these work:
if-then {greater 10 5} then {echo 'true'}
if-then {less-equal $score 100} then {echo 'valid score'}  
if-then {equal $a $b} then {echo 'equal values'}
if-then {not-equal $x 0} then {echo 'non-zero'}
```

## Combining with Arithmetic

The comparison operators work seamlessly with ES shell's arithmetic:

```bash
a = 3
b = 4
if-then {greater <={$a plus $b} 5} then {
    echo '3 + 4 is greater than 5'
}
```

## Implementation Details

The `if-then` function is implemented in ES shell itself using the existing `if` primitive and pattern matching. It supports:

- Simple `if-then condition then action` (no else clause)
- Full `if-then condition then action else action`
- Proper nesting and complex control flow
- Integration with all existing ES shell features

## Compatibility

This enhancement maintains full backward compatibility with existing ES shell code:

- Original `if` function still works as before
- All existing primitives and functions unchanged
- New comparison functions use standard shell conventions

## Testing

Run the demo scripts to see all features in action:

```bash
./es < demo-if-then.es
./es < comparison-demo.es
```

## Technical Notes

- Comparison functions return Unix exit status (0=success/true, 1=failure/false)
- The underlying primitives (`$&greater`, `$&less`, etc.) are implemented in C for performance
- Wrapper functions provide convenient shell-level access with proper status handling
- Floating-point comparisons use epsilon-based equality testing
- All operators work with both integer and floating-point numbers

This implementation provides a robust, familiar control structure while maintaining the unique power and flexibility of ES shell.