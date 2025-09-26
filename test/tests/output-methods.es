#!/usr/bin/env es

# Test if ~~ output can be captured in different ways
echo 'Test 1: Direct command output'
~~ hello h*

echo 'Test 2: Pipe to another command'  
~~ hello h* | wc -c

echo 'Test 3: Redirect to file'
{~~ hello h*} -> /tmp/test_output.txt
echo 'File contents:'
cat /tmp/test_output.txt
rm -f /tmp/test_output.txt

echo 'Test 4: Variable assignment'
result = `{~~ hello h*}
echo 'Result:' $result