#!/usr/bin/env es
# Simple debug test to trace tokenization
echo 'Testing simple case:'
echo ${5}

echo 'Testing with space:'  
echo ${5 + 0}

echo 'Testing without space:'
echo ${5+0} 2>&1 || echo 'Failed as expected'