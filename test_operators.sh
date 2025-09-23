#!/bin/bash
# Test script for new redirection and expression operators in es-shell

echo "=== Testing New ES-Shell Operators ==="
echo

# Create test files
echo "input data" > /tmp/es_test_input.txt
echo "second line" >> /tmp/es_test_input.txt

echo "1. Testing input redirection (<-)"
echo 'cat <- /tmp/es_test_input.txt' | ./es
echo

echo "2. Testing output redirection (->)"
echo 'echo "output test" -> /tmp/es_test_output.txt' | ./es
echo "Output file contents:"
cat /tmp/es_test_output.txt 2>/dev/null || echo "File not created or empty"
echo

echo "3. Testing append redirection (->>) "
echo 'echo "first line" -> /tmp/es_test_append.txt' | ./es
echo 'echo "second line" ->> /tmp/es_test_append.txt' | ./es
echo "Append file contents:"
cat /tmp/es_test_append.txt 2>/dev/null || echo "File not created or empty"
echo

echo "4. Testing expression evaluation syntax"
echo 'Testing ${...} parsing:'
echo 'result ${1}' | ./es 2>&1 || echo "Simple expression failed"
echo 'result ${true}' | ./es 2>&1 || echo "Boolean literal failed"  
echo

echo "5. Testing comparison operators (should parse without old redirection conflicts)"
echo 'Testing standalone comparison operators:'
echo 'if {1 < 2} {echo "less than works"}' | ./es 2>&1 || echo "Less than parsing failed"
echo 'if {2 > 1} {echo "greater than works"}' | ./es 2>&1 || echo "Greater than parsing failed"
echo

echo "6. Testing read-write redirection (<->)"  
echo 'echo "test data" -> /tmp/es_rw_test.txt' | ./es
echo 'sed s/test/modified/ <-> /tmp/es_rw_test.txt' | ./es 2>&1 || echo "Read-write failed"
echo "Read-write file contents:"
cat /tmp/es_rw_test.txt 2>/dev/null || echo "File not found"
echo

echo "7. Testing heredoc syntax (<--<)"
echo 'cat <--< EOF
This is a heredoc test
with multiple lines
EOF' | ./es 2>&1 || echo "Heredoc failed"
echo

echo "8. Testing word parsing with hyphens (should still work)"
echo 'echo "testing-hyphens-in-words"' | ./es
echo 'ls -la /tmp/nonexistent 2>/dev/null || echo "hyphen flags work"' | ./es
echo

# Cleanup
rm -f /tmp/es_test_*.txt /tmp/es_rw_test.txt

echo "=== Test Complete ==="