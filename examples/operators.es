# operators.es -- illustrate pattern matching, list operations, and boolean operators

# Pattern matching with ~ and its negation
if { ~ README.md R* } {
        echo match
} {
        echo no-match
}
if { ! ~ README.md z* } {
        echo no-match
} {
        echo match
}

# Pattern matching over lists
if { ~ (README.md COPYING) R* } {
        echo list-match
} {
        echo list-no-match
}

# List concatenation cross-product
a = (1 2)
b = (3 4)
echo concat: $a^$b

# Boolean operators and negation
fn log { echo ran:$1 }
false && { log left }
true || { log right }
false || { log after }
true && { log then }
! false && echo negated

# Pattern list against multiple patterns
if { ~ (foo bar) (f* b*) } {
        echo multi-pattern
} {
        echo multi-no
}

# Concatenation with an empty list yields nothing
empty = ()
echo with-empty: $a^$empty

# Capture outputs from logical operators
andout = `{ true && echo ok }
echo and-output: $andout
orout = `{ false || echo fallback }
echo or-output: $orout
