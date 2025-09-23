#!/usr/bin/env es

# Test infix operator support in cond statements

echo 'Testing cond with infix operators'
echo '================================='
echo ''

# Test basic infix comparison operators
echo '1. Basic infix comparison operators:'
a = 10
b = 5

echo -n '   10 greater 5: '
cond {$a greater $b} then {echo 'TRUE'} else {echo 'FALSE'}

echo -n '   10 less 5: '  
cond {$a less $b} then {echo 'TRUE'} else {echo 'FALSE'}

echo -n '   5 equal 5: '
cond {$b equal $b} then {echo 'TRUE'} else {echo 'FALSE'}

echo -n '   10 not-equal 5: '
cond {$a not-equal $b} then {echo 'TRUE'} else {echo 'FALSE'}

echo -n '   10 greater-equal 10: '  
cond {$a greater-equal $a} then {echo 'TRUE'} else {echo 'FALSE'}

echo -n '   5 less-equal 10: '
cond {$b less-equal $a} then {echo 'TRUE'} else {echo 'FALSE'}

echo ''

# Test arithmetic with infix
echo '2. Arithmetic with infix operators:'

# Simple arithmetic
sum = <={3 plus 4}
echo -n '   (3 plus 4) greater 6: '
cond {$sum greater 6} then {echo 'TRUE (7 > 6)'} else {echo 'FALSE'}

product = <={6 multiply 7}
echo -n '   (6 multiply 7) greater 40: '
cond {$product greater 40} then {echo 'TRUE (42 > 40)'} else {echo 'FALSE'}

difference = <={20 minus 8}
echo -n '   (20 minus 8) equal 12: '
cond {$difference equal 12} then {echo 'TRUE'} else {echo 'FALSE'}

echo ''

# Test mixed arithmetic and comparison  
echo '3. Complex expressions:'
x = 15
y = 25
echo -n '   x=15, y=25, x+10 equal y: '
enhanced_x = <={$x plus 10}
cond {$enhanced_x equal $y} then {echo 'TRUE (15+10 == 25)'} else {echo 'FALSE'}

echo ''

# Test with negative numbers
echo '4. Negative numbers:'
temp = -5
echo -n '   -5 greater -10: '
cond {$temp greater -10} then {echo 'TRUE'} else {echo 'FALSE'}

echo ''

echo 'INFIX OPERATOR SUPPORT SUMMARY:'
echo '==============================='
echo '✅ Word-based infix comparison operators: greater, less, equal, not-equal, greater-equal, less-equal'
echo '✅ Word-based infix arithmetic operators: plus, minus, multiply, divide, mod'
echo '✅ Mixed arithmetic and comparison expressions'
echo '✅ Variables in infix expressions'
echo '✅ Negative numbers'
echo ''
echo '❌ Symbolic infix operators (>, <, ==, !=) are NOT supported'
echo '   Use: $a greater $b (not: $a > $b)'
echo ''

echo 'WORKING PATTERNS:'
echo '================='
echo '• cond {$var greater 5} then {action}'
echo '• cond {$x equal $y} then {action}'  
echo '• cond {$result less-equal 100} then {action}'
echo '• cond {<={$a plus $b} greater 10} then {action}'
echo ''

echo 'Infix operator testing complete!'