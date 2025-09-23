#!/bin/bash
# Extensive test suite for ES-Shell new syntax implementation

echo "=== EXTENSIVE ES-SHELL SYNTAX TESTING ==="
echo

PASSED=0
FAILED=0

# Test helper function
test_command() {
    local description="$1"
    local command="$2"
    local expected="$3"
    local test_type="${4:-output}"
    
    echo -n "Testing: $description... "
    
    if [[ "$test_type" == "output" ]]; then
        result=$(echo "$command" | ./es 2>/dev/null)
        if [[ "$result" == "$expected" ]]; then
            echo "✅ PASS"
            ((PASSED++))
        else
            echo "❌ FAIL"
            echo "  Expected: '$expected'"
            echo "  Got: '$result'"
            ((FAILED++))
        fi
    elif [[ "$test_type" == "file" ]]; then
        echo "$command" | ./es >/dev/null 2>&1
        if [[ -f "$expected" ]]; then
            content=$(cat "$expected" 2>/dev/null)
            echo "✅ PASS (file created with: '$content')"
            ((PASSED++))
        else
            echo "❌ FAIL (file not created: $expected)"
            ((FAILED++))
        fi
    elif [[ "$test_type" == "file_content" ]]; then
        local filepath="$expected"
        local expected_content="$4"
        echo "$command" | ./es >/dev/null 2>&1
        if [[ -f "$filepath" ]]; then
            content=$(cat "$filepath" 2>/dev/null)
            if [[ "$content" == "$expected_content" ]]; then
                echo "✅ PASS"
                ((PASSED++))
            else
                echo "❌ FAIL"
                echo "  Expected content: '$expected_content'"
                echo "  Got content: '$content'"
                ((FAILED++))
            fi
        else
            echo "❌ FAIL (file not created: $filepath)"
            ((FAILED++))
        fi
    fi
}

echo "=== 1. BASIC FUNCTIONALITY TESTS ==="

test_command "Basic echo" 'echo hello' 'hello'
test_command "External command ls" 'ls /dev/null' '/dev/null'
test_command "Variable assignment" 'x=test; echo $x' 'test'
test_command "Command substitution" 'echo ${echo test}' 'test
0'

echo
echo "=== 2. REDIRECTION OPERATOR TESTS ==="

# Clean up any existing test files
rm -f /tmp/es_test_*

test_command "Input redirection" 'echo "input test" > /tmp/es_test_input.txt && cat <- /tmp/es_test_input.txt' '"input test"'

# Output redirection with external commands
echo "/bin/echo 'output test' -> /tmp/es_test_output.txt" | ./es >/dev/null 2>&1
test_command "Output redirection creates file" "" "/tmp/es_test_output.txt" "file"

# Test file content for output redirection  
test_command "Output redirection content" "/bin/echo 'content test' -> /tmp/es_test_out2.txt" "/tmp/es_test_out2.txt" "content test"

# Append redirection
echo "/bin/echo 'line1' -> /tmp/es_test_append.txt" | ./es >/dev/null 2>&1
echo "/bin/echo 'line2' ->> /tmp/es_test_append.txt" | ./es >/dev/null 2>&1
content=$(cat /tmp/es_test_append.txt 2>/dev/null)
if [[ "$content" == "line1line2" ]] || [[ "$content" == "line1\nline2" ]]; then
    echo "Testing: Append redirection... ✅ PASS"
    ((PASSED++))
else
    echo "Testing: Append redirection... ❌ FAIL (got: '$content')"
    ((FAILED++))
fi

# Heredoc test
result=$(echo 'cat <--< EOF
heredoc line 1
heredoc line 2
EOF' | ./es 2>/dev/null)
if [[ "$result" == *"heredoc line 1"* && "$result" == *"heredoc line 2"* ]]; then
    echo "Testing: Heredoc syntax... ✅ PASS"
    ((PASSED++))
else
    echo "Testing: Heredoc syntax... ❌ FAIL"
    ((FAILED++))
fi

echo
echo "=== 3. COMPARISON OPERATORS - COMPREHENSIVE TESTS ==="

# Basic comparison tests
test_command "Less than (true case)" 'if {1 < 2} {echo "true"} {echo "false"}' '"true"'
test_command "Less than (false case)" 'if {2 < 1} {echo "true"} {echo "false"}' '"false"'
test_command "Greater than (true case)" 'if {3 > 2} {echo "true"} {echo "false"}' '"true"'  
test_command "Greater than (false case)" 'if {2 > 3} {echo "true"} {echo "false"}' '"false"'
test_command "Equal (true case)" 'if {5 == 5} {echo "true"} {echo "false"}' '"true"'
test_command "Equal (false case)" 'if {5 == 4} {echo "true"} {echo "false"}' '"false"'
test_command "Not equal (true case)" 'if {5 != 4} {echo "true"} {echo "false"}' '"true"'
test_command "Not equal (false case)" 'if {4 != 4} {echo "true"} {echo "false"}' '"false"'

# Comprehensive conditional tests as requested
test_command "Condition: 5 != 4 (should be true)" 'if {5 != 4} {echo "good condition"} {echo "bad condition"}' '"good condition"'
test_command "Condition: 5 == 4 (should be false)" 'if {5 == 4} {echo "bad condition"} {echo "good condition"}' '"good condition"'

# More complex comparison tests
test_command "Less than equal (true)" 'if {3 <= 3} {echo "true"} {echo "false"}' '"true"'
test_command "Less than equal (false)" 'if {4 <= 3} {echo "true"} {echo "false"}' '"false"' 
test_command "Greater than equal (true)" 'if {5 >= 4} {echo "true"} {echo "false"}' '"true"'
test_command "Greater than equal (false)" 'if {3 >= 4} {echo "true"} {echo "false"}' '"false"'

echo
echo "=== 4. ARITHMETIC OPERATORS - COMPREHENSIVE TESTS ==="

test_command "Addition" 'x = ${2 + 3}; echo $x' '5'
test_command "Subtraction" 'x = ${10 - 4}; echo $x' '6'
test_command "Multiplication (word form)" 'x = ${4 times 5}; echo $x' '20'
test_command "Division" 'x = ${20 / 4}; echo $x' '5'
test_command "Modulo" 'x = ${13 mod 5}; echo $x' '3'

# Test operator precedence
test_command "Precedence: 2 + 3 * 4" 'x = ${2 + 3 times 4}; echo $x' '14'
test_command "Precedence: 10 - 6 / 2" 'x = ${10 - 6 / 2}; echo $x' '7'

# Test complex expressions
test_command "Complex: (5 + 3) > 7" 'if {${5 + 3} > 7} {echo "true"} {echo "false"}' '"true"'
test_command "Complex: (2 * 3) == 6" 'if {${2 times 3} == 6} {echo "true"} {echo "false"}' '"true"'

echo
echo "=== 5. EXPRESSION EVALUATION TESTS ==="

test_command "Variable from expression" 'result = ${7 + 8}; echo "result is $result"' '"result is 15"'
test_command "Nested expressions" 'a = ${3 + 2}; b = ${a times 2}; echo $b' '10'
test_command "Expression in condition" 'if {${10 / 2} == 5} {echo "math works"}' '"math works"'

echo
echo "=== 6. MIXED SYNTAX COMPATIBILITY TESTS ==="

test_command "Old and new together" 'x = ${1 + 1}; echo $x -> /tmp/es_mixed.txt; cat <- /tmp/es_mixed.txt' '2'
test_command "Hyphens in words" 'echo "multi-word-test"' '"multi-word-test"'
test_command "Commands with flags" 'ls --version >/dev/null 2>&1 || echo "flag test passed"' '"flag test passed"'

echo  
echo "=== 7. EDGE CASE TESTS ==="

test_command "Zero comparisons" 'if {0 == 0} {echo "zero equal"} {echo "zero fail"}' '"zero equal"'
test_command "Negative numbers" 'x = ${5 + -3}; echo $x' '2'
test_command "Floating point" 'x = ${5.5 + 1.5}; echo $x' '7'

# Test that old problematic syntax doesn't work
result=$(echo 'echo test > output.txt' | ./es 2>&1)
if [[ "$result" == *"syntax error"* ]] || [[ ! -f "output.txt" ]]; then
    echo "Testing: Old > syntax properly disabled... ✅ PASS"
    ((PASSED++))
else
    echo "Testing: Old > syntax properly disabled... ❌ FAIL"
    ((FAILED++))
fi

echo
echo "=== 8. ROBUST FUNCTIONALITY TESTS ==="

# Test multiple redirections
echo "/bin/echo 'multi1' -> /tmp/es_multi1.txt" | ./es >/dev/null 2>&1
echo "/bin/echo 'multi2' -> /tmp/es_multi2.txt" | ./es >/dev/null 2>&1
content1=$(cat /tmp/es_multi1.txt 2>/dev/null)
content2=$(cat /tmp/es_multi2.txt 2>/dev/null) 
if [[ "$content1" == "multi1" && "$content2" == "multi2" ]]; then
    echo "Testing: Multiple separate redirections... ✅ PASS"
    ((PASSED++))
else
    echo "Testing: Multiple separate redirections... ❌ FAIL"
    ((FAILED++))
fi

# Test chained operations
test_command "Chained operations" 'x = ${2 + 3}; y = ${x times 2}; if {y > 9} {echo "chain works"}' '"chain works"'

# Cleanup test files
rm -f /tmp/es_test_* /tmp/es_multi*.txt /tmp/es_mixed.txt output.txt

echo
echo "=== TEST RESULTS SUMMARY ==="
echo "✅ Tests Passed: $PASSED"
echo "❌ Tests Failed: $FAILED"
echo "📊 Success Rate: $(echo "scale=1; $PASSED * 100 / ($PASSED + $FAILED)" | bc -l)%"

if [[ $FAILED -eq 0 ]]; then
    echo
    echo "🎉 ALL TESTS PASSED! Implementation is ready for production."
    exit 0
else
    echo
    echo "⚠️  Some tests failed. Please review before proceeding."
    exit 1
fi