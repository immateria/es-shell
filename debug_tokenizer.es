#!/usr/bin/env es
# Debug script to test tokenization behavior
echo 'Testing simple cases:'
echo 'With spaces: ${5 + 0}'
echo ${5 + 0}
echo 'Without spaces: ${5+0}'
echo ${5+0} 2>&1 || echo 'Failed as expected'

echo 
echo 'Testing other operators:'
echo 'Multiplication: ${5*3}'  
echo ${5*3} 2>&1 || echo 'Failed'
echo 'Subtraction: ${5-3}'
echo ${5-3} 2>&1 || echo 'Failed'

echo
echo 'Testing if it works when operators start words:'
echo 'Plus at start: ${+5}'
echo ${+5} 2>&1 || echo 'Failed'
echo 'Star at start: ${*}'
echo ${*} 2>&1 || echo 'Failed (expected)'