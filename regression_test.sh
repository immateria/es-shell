#!/bin/bash
# ES Shell Regression Test
# Run from shell (not ES shell) to test ES shell functionality

cd /Users/immateria/ES-SHELL/es-shell

echo "=== ES SHELL REGRESSION TEST ==="
echo "Testing after ${} literal number fix"
echo

# Function to run test
run_test() {
    local name="$1"
    local command="$2"
    local expected="$3"
    
    echo -n "Testing $name: "
    result=$(./es -c "$command" 2>/dev/null)
    if [ "$result" = "$expected" ]; then
        echo "✓ PASS"
        ((tests_passed++))
    else
        echo "✗ FAIL (expected: '$expected', got: '$result')"
        ((tests_failed++))
        failed_tests+=("$name")
    fi
}

# Function to run command test (just check if it succeeds)
run_cmd_test() {
    local name="$1"
    local command="$2"
    
    echo -n "Testing $name: "
    if ./es -c "$command" >/dev/null 2>&1; then
        echo "✓ PASS"
        ((tests_passed++))
    else
        echo "✗ FAIL"
        ((tests_failed++))
        failed_tests+=("$name")
    fi
}

tests_passed=0
tests_failed=0
failed_tests=()

echo "1. ARITHMETIC EXPRESSIONS"
echo "========================="

# Literal numbers 
run_test "literal positive" 'echo ${5}' '5'
run_test "literal negative" 'echo ${-3}' '-3'
run_test "literal float" 'echo ${3.14}' '3.14'
run_test "literal zero" 'echo ${0}' '0'

# Compound arithmetic
run_test "compound addition" 'echo ${5+3}' '8'
run_test "compound subtraction" 'echo ${10-4}' '6'
run_test "compound multiplication" 'echo ${3*4}' '12'
run_test "compound division" 'echo ${15/3}' '5'

# Spaced arithmetic  
run_test "spaced addition" 'echo ${5 + 3}' '8'
run_test "spaced subtraction" 'echo ${10 - 4}' '6'
run_test "spaced multiplication" 'echo ${3 * 4}' '12'
run_test "spaced division" 'echo ${15 / 3}' '5'

echo
echo "2. BASIC FUNCTIONALITY"
echo "======================"

# Basic commands
run_test "echo command" 'echo hello' 'hello'
run_test "echo with spaces" 'echo hello world' 'hello world'

# Variables
run_test "variable assignment" 'x = 42; echo $x' '42'
run_test "variable arithmetic" 'x = 5; y = 3; echo ${$x + $y}' '8'

echo  
echo "3. I/O AND PIPES"
echo "================"

# Create temp file for testing
tmpfile="/tmp/es-test-$$"
echo "test-content" > "$tmpfile"

# File I/O
run_test "file read" "cat $tmpfile" 'test-content'
run_test "simple pipe" 'echo hello | cat' 'hello'

# Cleanup
rm -f "$tmpfile"

echo
echo "4. CONDITIONALS"
echo "==============="

# Pattern matching
run_test "pattern match true" 'if {~ hello hello} {echo true} {echo false}' 'true'
run_test "pattern match false" 'if {~ hello world} {echo true} {echo false}' 'false'

echo
echo "5. EXTERNAL COMMANDS"  
echo "===================="

# External command tests
run_cmd_test "external echo" '/bin/echo test'
run_cmd_test "external true" '/usr/bin/true' 
run_cmd_test "path resolution" 'true'

echo
echo "=== TEST SUMMARY ==="
total_tests=$((tests_passed + tests_failed))
echo "Tests passed: $tests_passed"
echo "Tests failed: $tests_failed" 
echo "Total tests:  $total_tests"

if [ $tests_failed -eq 0 ]; then
    echo "✓ ALL TESTS PASSED - ES Shell is working correctly!"
    exit 0
else
    echo "✗ FAILED TESTS:"
    for test in "${failed_tests[@]}"; do
        echo "  - $test"
    done
    echo
    echo "Please investigate failed tests."
    exit 1
fi