#!/usr/bin/env es
# Test overflow behavior with very large numbers
echo '=== Testing Overflow Behavior ==='

echo 'Testing very large multiplication (should overflow to float):'
echo ${9223372036854775000 × 2}

echo 'Testing negative overflow:'  
echo ${-9223372036854775000 × 2}

echo 'Testing addition overflow:'
echo ${9223372036854775800 + 100}

echo 'Testing negative addition overflow:'
echo ${-9223372036854775800 + (-100)}

echo 'Testing mixed operations that should stay integer:'
echo ${1000000 + (-500000)}
echo ${-2000000 × 2}

echo '=== Overflow Tests Complete ==='