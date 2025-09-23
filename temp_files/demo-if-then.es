#!/usr/bin/env es

# Demo of if-then control structure with comparison operators
echo 'ES Shell if-then and comparison operators demo'
echo '=============================================='

# Test basic comparisons
x = 10
y = 5

echo 'Testing x=10 and y=5:'
if-then {greater $x $y} then {
    echo '  x is greater than y'
} else {
    echo '  x is not greater than y'
}

if-then {less $x $y} then {
    echo '  x is less than y'
} else {
    echo '  x is not less than y'
}

if-then {equal $x $y} then {
    echo '  x equals y'
} else {
    echo '  x does not equal y'
}

# Test with equal values
echo ''
echo 'Testing x=7 and y=7:'
x = 7
y = 7
if-then {equal $x $y} then {
    echo '  x equals y'
} else {
    echo '  x does not equal y'
}

# Demonstrate simple if-then without else
echo ''
echo 'Testing simple if-then (no else):'
score = 95
if-then {greater-equal $score 90} then {
    echo '  Excellent score!'
}

score = 75
if-then {greater-equal $score 90} then {
    echo '  Excellent score!'
}
echo '  (No output expected for score=75)'

# Test arithmetic expressions within comparisons
echo ''
echo 'Testing arithmetic in comparisons:'
a = 3
b = 4
if-then {greater <={$a plus $b} 5} then {
    echo '  3 + 4 is greater than 5'
} else {
    echo '  3 + 4 is not greater than 5'
}

echo ''
echo 'Demo complete!'