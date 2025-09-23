#!/usr/bin/env es

# Comprehensive demonstration of comparison operators and if-then syntax

echo 'ES Shell Comparison Operators Demo'
echo '=================================='
echo ''

# Test all comparison functions
num1 = 10
num2 = 5
num3 = 10

echo 'Testing with num1=10, num2=5, num3=10:'
echo ''

echo '1. Testing greater:'
if-then {greater $num1 $num2} then {
    echo '  10 > 5: TRUE'
} else {
    echo '  10 > 5: FALSE'
}

echo ''
echo '2. Testing less:'
if-then {less $num1 $num2} then {
    echo '  10 < 5: TRUE'
} else {
    echo '  10 < 5: FALSE'
}

echo ''
echo '3. Testing greater-equal:'
if-then {greater-equal $num1 $num3} then {
    echo '  10 >= 10: TRUE'
} else {
    echo '  10 >= 10: FALSE'
}

echo ''
echo '4. Testing less-equal:'
if-then {less-equal $num2 $num1} then {
    echo '  5 <= 10: TRUE'
} else {
    echo '  5 <= 10: FALSE'
}

echo ''
echo '5. Testing equal:'
if-then {equal $num1 $num3} then {
    echo '  10 == 10: TRUE'
} else {
    echo '  10 == 10: FALSE'
}

echo ''
echo '6. Testing not-equal:'
if-then {not-equal $num1 $num2} then {
    echo '  10 != 5: TRUE'
} else {
    echo '  10 != 5: FALSE'
}

echo ''
echo '7. Complex example - grade calculation:'
echo ''

# Grade calculation function using nested if-then
score = 87
echo 'Score:' $score

if-then {greater-equal $score 90} then {
    echo 'Grade: A (Excellent!)'
} else {
    if-then {greater-equal $score 80} then {
        echo 'Grade: B (Good work!)'
    } else {
        if-then {greater-equal $score 70} then {
            echo 'Grade: C (Satisfactory)'
        } else {
            if-then {greater-equal $score 60} then {
                echo 'Grade: D (Needs improvement)'
            } else {
                echo 'Grade: F (Failing)'
            }
        }
    }
}

echo ''
echo 'Demo complete! All comparison operators are working.'