#!/usr/bin/env es

# Test script for the new cond elseif syntax

echo 'Testing cond with elseif syntax'
echo '================================='

echo ''
echo '1. Basic if-then-else (score=85):'
score = 85
cond {greater $score 90} then {echo 'A'} else {echo 'not A'}

echo ''
echo '2. If-then-elseif-else chain (score=85):'
score = 85  
cond {greater $score 90} then {echo 'A'} elseif {greater $score 80} then {echo 'B'} else {echo 'C'}

echo ''
echo '3. If-then-elseif-else chain (score=95):'
score = 95
cond {greater $score 90} then {echo 'A'} elseif {greater $score 80} then {echo 'B'} else {echo 'C'}

echo ''
echo '4. If-then-elseif-else chain (score=75):'
score = 75
cond {greater $score 90} then {echo 'A'} elseif {greater $score 80} then {echo 'B'} else {echo 'C'}

echo ''
echo '5. If-then-elseif (no else, score=85):'
score = 85
cond {greater $score 90} then {echo 'A'} elseif {greater $score 80} then {echo 'B'}

echo ''
echo 'Tests complete!'