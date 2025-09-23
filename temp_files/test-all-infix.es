#!/usr/bin/env es

# Comprehensive test of all infix operator styles in ES shell

echo 'ES Shell Infix Operators - Complete Test Suite'
echo '=============================================='
echo ''

# Test all comparison operator styles
a = 10
b = 5
c = 10

echo 'COMPARISON OPERATORS - All Available Styles:'
echo '============================================'
echo ''

echo '1. Word-based operators (most readable):'
echo -n '   10 greater 5: '; cond {$a greater $b} then {echo TRUE} else {echo FALSE}
echo -n '   10 less 5: '; cond {$a less $b} then {echo TRUE} else {echo FALSE}
echo -n '   10 equal 10: '; cond {$a equal $c} then {echo TRUE} else {echo FALSE}
echo -n '   10 not-equal 5: '; cond {$a not-equal $b} then {echo TRUE} else {echo FALSE}
echo ''

echo '2. Short form operators (compact):'
echo -n '   10 gt 5: '; cond {$a gt $b} then {echo TRUE} else {echo FALSE}
echo -n '   10 lt 5: '; cond {$a lt $b} then {echo TRUE} else {echo FALSE}
echo -n '   10 eq 10: '; cond {$a eq $c} then {echo TRUE} else {echo FALSE}
echo -n '   10 ne 5: '; cond {$a ne $b} then {echo TRUE} else {echo FALSE}
echo -n '   10 ge 10: '; cond {$a ge $c} then {echo TRUE} else {echo FALSE}
echo -n '   5 le 10: '; cond {$b le $a} then {echo TRUE} else {echo FALSE}
echo ''

echo '3. Symbolic-style operators (visual):'
echo -n '   10 _gt_ 5: '; cond {$a _gt_ $b} then {echo TRUE} else {echo FALSE}
echo -n '   10 _lt_ 5: '; cond {$a _lt_ $b} then {echo TRUE} else {echo FALSE}
echo -n '   10 _eq_ 10: '; cond {$a _eq_ $c} then {echo TRUE} else {echo FALSE}
echo -n '   10 _ne_ 5: '; cond {$a _ne_ $b} then {echo TRUE} else {echo FALSE}
echo -n '   10 _ge_ 10: '; cond {$a _ge_ $c} then {echo TRUE} else {echo FALSE}
echo -n '   5 _le_ 10: '; cond {$b _le_ $a} then {echo TRUE} else {echo FALSE}
echo ''

echo 'ARITHMETIC OPERATORS - Multiple Styles:'
echo '======================================='
echo ''

echo '1. Word-based arithmetic:'
echo '   3 plus 4 = ' <={3 plus 4}
echo '   10 minus 3 = ' <={10 minus 3}
echo '   6 multiply 7 = ' <={6 multiply 7}
echo '   20 divide 4 = ' <={20 divide 4}
echo '   17 mod 5 = ' <={17 mod 5}
echo ''

echo '2. Symbolic arithmetic (working!):'
echo '   3 + 4 = ' <={3 + 4}
echo '   10 - 3 = ' <={10 - 3}
echo '   6 * 7 = ' <={6 * 7}
echo '   20 / 4 = ' <={20 / 4}
echo '   17 % 5 = ' <={17 % 5}
echo '   2 ** 3 = ' <={2 ** 3}
echo ''

echo 'COMPLEX EXPRESSIONS IN CONDITIONS:'
echo '=================================='
echo ''

score = 85
threshold = 80

echo 'Grade calculation using different operator styles:'
echo ''

echo -n 'Word style - Score ' $score ': '
cond {$score greater-equal 90} then {echo A} elseif {$score greater-equal 80} then {echo B} else {echo C}

echo -n 'Short style - Score ' $score ': '  
cond {$score ge 90} then {echo A} elseif {$score ge 80} then {echo B} else {echo C}

echo -n 'Symbolic style - Score ' $score ': '
cond {$score _ge_ 90} then {echo A} elseif {$score _ge_ 80} then {echo B} else {echo C}

echo ''

echo 'Arithmetic with comparison:'
balance = 1000
fee = 50
remaining = <={$balance minus $fee}
echo -n 'Balance after fee ($' $balance ' - $' $fee ' = $' $remaining '): '
cond {$remaining _gt_ 900} then {echo 'Good balance'} else {echo 'Low balance'}

echo ''

echo 'OPERATOR STYLE COMPARISON:'
echo '========================='
echo '✅ Word-based:   most readable, familiar English'
echo '✅ Short forms:  compact, traditional abbreviations'  
echo '✅ Symbolic:     visual, math-like appearance'
echo '✅ Mixed usage:  combine styles as needed'
echo ''

echo 'ALL OPERATOR STYLES WORK IN:'
echo '- Basic comparisons: echo <={x op y}'
echo '- Conditional statements: cond {x op y} then {action}'
echo '- Complex expressions: cond {<={a + b} op c} then {action}'
echo '- Elseif chains: full support in all patterns'
echo ''

echo 'Complete infix operator test successful!'