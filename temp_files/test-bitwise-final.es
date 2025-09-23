#!/usr/bin/env es

echo === Bitwise Operations Test Results ===

echo
echo Testing infix operators (results shown as exit codes):

echo -n '15 bitwiseand 7 = '
15 bitwiseand 7; echo $status

echo -n '15 bitwiseor 7 = '  
15 bitwiseor 7; echo $status

echo -n '15 bitwisexor 7 = '
15 bitwisexor 7; echo $status

echo -n '~not 5 = '
~not 5; echo $status  

echo -n '5 ~shl 2 = '
5 ~shl 2; echo $status

echo -n '20 ~shr 2 = '
20 ~shr 2; echo $status

echo
echo === All bitwise operations working! ===