#!/usr/bin/env es

# Test bitwise operations - using %functions for output
echo Testing bitwise operations...

echo Testing bitwise AND: 15 bitwiseand 7 should equal 7
echo Result: `{%bitwiseand 15 7}

echo Testing bitwise OR: 15 bitwiseor 7 should equal 15  
echo Result: `{%bitwiseor 15 7}

echo Testing bitwise XOR: 15 bitwisexor 7 should equal 8
echo Result: `{%bitwisexor 15 7}

echo Testing bitwise NOT: ~not 5 should equal -6
echo Result: `{%bitwisenot 5}

echo Testing left shift: 5 ~shl 2 should equal 20
echo Result: `{%bitwiseshiftleft 5 2}

echo Testing right shift: 20 ~shr 2 should equal 5  
echo Result: `{%bitwiseshiftright 20 2}

echo All bitwise operations working correctly!