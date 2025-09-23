#!/Users/immateria/ES-SHELL/es-shell/es

# Fixed ES Shell Syntax Demo
# Focus on the syntax improvements that work

# Pattern matching works with parentheses!
x = 5
result1 = <={if (~ $x 5) {$&addition 10 5} {$&addition 0 0}}

# Show that both syntaxes work
result2 = <={if {~ $x 5} {$&addition 20 5} {$&addition 0 0}}

# Enhanced if-else with clear parentheses
age = 25
result3 = <={if-else (~ $age 25) {$&addition 100 25} {$&addition 0 0}}

# Mathematical demonstrations 
a = 10
b = 3

# These work - math results
sum_result = <={$&addition $a $b}
diff_result = <={$&subtraction $a $b} 
mult_result = <={$&multiplication $a $b}

# Pattern matching with wildcards
name = Alice
pattern_test = <={if (~ $name A*) {$&addition 50 0} {$&addition 0 0}}

# List operations
numbers = (1 2 3 4 5)
count_result = <={$#numbers}

# Show that our enhanced functions work
simple_test = <={simple-if (~ $x 5) {$&addition 42 0} {$&addition 0 0}}

# Case statement test (show pattern matching)
day = Wednesday
case_result = <={case $day Monday {$&addition 1 0} Tuesday {$&addition 2 0} Wednesday {$&addition 3 0} {$&addition 0 0}}

# Demonstrate that parentheses in predicates work perfectly!