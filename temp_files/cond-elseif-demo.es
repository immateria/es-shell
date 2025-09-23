#!/usr/bin/env es

# Comprehensive demo of the new cond elseif functionality
# This eliminates the need for nested if statements!

echo 'ES Shell Enhanced Conditional Syntax Demo'
echo '=========================================='
echo ''

echo 'Before: Nested if statements were required:'
echo '  if {condition1} {'
echo '      action1' 
echo '  } {'
echo '      if {condition2} {'
echo '          action2'
echo '      } {'
echo '          action3'
echo '      }'
echo '  }'
echo ''

echo 'Now: Flat conditional chains with elseif!'
echo '  cond {condition1} then {action1} elseif {condition2} then {action2} else {action3}'
echo ''

echo 'DEMONSTRATIONS:'
echo '================'
echo ''

# Demo 1: Grade calculator with single elseif
echo '1. Grade Calculator (with elseif):'
for (score = 95 85 75 65) {
    echo -n '   Score' $score ': '
    cond {greater-equal $score 90} then {
        echo 'A (Excellent!)'
    } elseif {greater-equal $score 80} then {
        echo 'B (Good work!)'  
    } else {
        echo 'C or below'
    }
}
echo ''

# Demo 2: Simple binary choice
echo '2. Simple Binary Decision:'
for (age = 25 15) {
    echo -n '   Age' $age ': '
    cond {greater-equal $age 18} then {
        echo 'Adult'
    } else {
        echo 'Minor'
    }
}
echo ''

# Demo 3: Just if-then (no else)
echo '3. Simple If-Then (no else needed):'
balance = 1000
echo -n '   Balance $' $balance ': '
cond {greater $balance 500} then {
    echo 'Account is in good standing'
}
echo '   (no output expected if condition was false)'
echo ''

# Demo 4: Using with different comparison operators
echo '4. Various Comparison Operators:'
echo -n '   10 == 10: '
cond {equal 10 10} then {echo 'Equal!'} else {echo 'Not equal'}

echo -n '   15 != 20: '
cond {not-equal 15 20} then {echo 'Different!'} else {echo 'Same'} 

echo -n '   7 <= 10: '
cond {less-equal 7 10} then {echo 'Less or equal!'} else {echo 'Greater'}
echo ''

# Demo 5: Complex arithmetic in conditions  
echo '5. Arithmetic in Conditions:'
x = 3
y = 4
echo -n '   Is (3+4) > 6? '
cond {greater <={$x plus $y} 6} then {
    echo 'Yes, 3+4=7 > 6'
} else {
    echo 'No, 3+4=7 <= 6'
}
echo ''

echo 'KEY BENEFITS:'
echo '============='
echo '• Eliminates deeply nested if statements'  
echo '• More readable than nested conditionals'
echo '• Familiar syntax from other programming languages'
echo '• Works with all ES shell comparison operators'
echo '• Supports arithmetic expressions in conditions'
echo '• Maintains full ES shell functionality and power'
echo ''

echo 'SYNTAX PATTERNS SUPPORTED:'
echo '=========================='
echo '  cond {condition} then {action}'
echo '  cond {condition} then {action} else {action}' 
echo '  cond {condition} then {action} elseif {condition} then {action}'
echo '  cond {condition} then {action} elseif {condition} then {action} else {action}'
echo ''

echo 'Demo complete! ES shell now has modern conditional syntax.'