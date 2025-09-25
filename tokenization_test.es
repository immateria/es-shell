#!/usr/bin/env es
# Comprehensive tokenization test to understand the exact behavior
echo '=== Tokenization Analysis ==='

echo 
echo 'WORKING CASES (with spaces):'
echo 'Test: echo ${5 + 0}'
echo ${5 + 0}

echo 'Test: echo ${5 * 3}'  
echo ${5 * 3}

echo 'Test: echo ${5 - 2}'
echo ${5 - 2}

echo
echo 'FAILING CASES (without spaces):'
echo 'Test: echo ${5+0}'
echo ${5+0} 2>&1 || echo '  -> FAILED (as expected)'

echo 'Test: echo ${5*3}'
echo ${5*3} 2>&1 || echo '  -> FAILED (as expected)'

echo 'Test: echo ${5-2}'
echo ${5-2} 2>&1 || echo '  -> FAILED (as expected)'

echo
echo 'EDGE CASES:'
echo 'Test: echo ${+5}'  # operator at start
echo ${+5} 2>&1 || echo '  -> FAILED'

echo 'Test: echo ${*abc}'  # operator with letters
echo ${*abc} 2>&1 || echo '  -> FAILED (expected)'

echo 'Test: echo ${5}+${0}'  # separate expressions
echo ${5}+${0}

echo
echo 'CONTROL TESTS (non-arithmetic):'
echo 'Test: echo *'  # glob pattern
echo *

echo 'Test: echo foo+bar'  # plus in regular context
echo foo+bar

echo '=== Analysis Complete ==='