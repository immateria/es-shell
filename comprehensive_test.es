#!/usr/local/bin/es
# ES Shell Comprehensive Regression Test
# Tests all major functionality to ensure nothing is broken

echo "=== ES SHELL COMPREHENSIVE REGRESSION TEST ==="
echo "Testing ES shell functionality after ${} literal number fix..."
echo

# Test counters
tests_passed = 0
tests_failed = 0
failed_tests = ()

# Helper functions
fn-test = @ name command expected {
    echo -n "Testing $name: "
    result = `{ $command }
    if {~ $result $expected} {
        echo "✓ PASS"
        tests_passed = `{%addition $tests_passed 1}
    } {
        echo "✗ FAIL (expected: $expected, got: $result)"
        tests_failed = `{%addition $tests_failed 1}
        failed_tests = $failed_tests $name
    }
}

fn-test-cmd = @ name command {
    echo -n "Testing $name: "
    if {$command >/dev/null 2>/dev/null} {
        echo "✓ PASS"
        tests_passed = `{%addition $tests_passed 1}
    } {
        echo "✗ FAIL"
        tests_failed = `{%addition $tests_failed 1}
        failed_tests = $failed_tests $name
    }
}

echo "1. ARITHMETIC EXPRESSIONS"
echo "========================="

# Basic literal numbers
test 'literal positive' 'echo ${5}' 5
test 'literal negative' 'echo ${-3}' -3
test 'literal float' 'echo ${3.14}' 3.14
test 'literal zero' 'echo ${0}' 0

# Compound arithmetic
test 'compound addition' 'echo ${5+3}' 8
test 'compound subtraction' 'echo ${10-4}' 6
test 'compound multiplication' 'echo ${3*4}' 12
test 'compound division' 'echo ${15/3}' 5

# Spaced arithmetic
test 'spaced addition' 'echo ${5 + 3}' 8
test 'spaced subtraction' 'echo ${10 - 4}' 6
test 'spaced multiplication' 'echo ${3 * 4}' 12
test 'spaced division' 'echo ${15 / 3}' 5

# Arithmetic primitives
test 'addition primitive' 'echo `{%addition 5 3}' 8
test 'subtraction primitive' 'echo `{%subtraction 10 4}' 6
test 'multiplication primitive' 'echo `{%multiplication 3 4}' 12
test 'division primitive' 'echo `{%division 15 3}' 5

echo
echo "2. VARIABLES AND ASSIGNMENT"
echo "==========================="

# Variable assignment and lookup
x = 42
test 'variable assignment' 'echo $x' 42
test 'variable in expression' 'echo ${$x}' 42

# Multiple variables
a = 5; b = 3
test 'multiple variables' 'echo `{%addition $a $b}' 8

echo
echo "3. BASIC I/O AND COMMANDS"  
echo "========================="

# Echo command
test 'echo command' 'echo hello' hello
test 'echo with spaces' 'echo hello world' 'hello world'

# Create temporary files for I/O tests
tmpfile = /tmp/es-test-$$
echo test-content > $tmpfile

# File operations
test 'file creation' 'cat $tmpfile' test-content
test-cmd 'file removal' 'rm $tmpfile'

echo
echo "4. CONDITIONALS AND CONTROL FLOW"
echo "================================="

# Pattern matching  
test 'pattern match true' 'if {~ hello hello} {echo true} {echo false}' true
test 'pattern match false' 'if {~ hello world} {echo true} {echo false}' false

# Numeric comparisons
test 'numeric equal' 'if {~ 5 5} {echo true} {echo false}' true
test 'numeric not equal' 'if {~ 5 3} {echo true} {echo false}' false

echo
echo "5. EXTERNAL COMMANDS AND PATH"
echo "=============================="

# Test external commands that should exist on most systems
test-cmd 'external /bin/echo' '/bin/echo hello'
test-cmd 'external /usr/bin/true' '/usr/bin/true'
test-cmd 'path resolution' 'true'  # Should find true in PATH

echo  
echo "6. PIPES AND COMMAND SUBSTITUTION"
echo "=================================="

# Pipes
test 'simple pipe' 'echo hello | cat' hello
test 'pipe with wc' 'echo hello | wc -w' 1

# Command substitution
test 'command substitution' 'echo `{echo nested}' nested

echo
echo "7. FUNCTIONS"
echo "============"

# Function definition and calling
fn-double = @ x {%multiplication $x 2}
test 'function definition' 'double 5' 10

echo
echo "8. ADVANCED FEATURES"
echo "===================="

# Lists and iteration  
test 'list creation' 'echo `{%count a b c}' 3

# Error handling
test-cmd 'error handling' '{nonexistent-command 2>/dev/null; true}'

echo
echo "=== REGRESSION TEST SUMMARY ==="
echo "Tests passed: $tests_passed"
echo "Tests failed: $tests_failed"
total_tests = `{%addition $tests_passed $tests_failed}
echo "Total tests:  $total_tests"

if {~ $tests_failed 0} {
    echo "✓ ALL TESTS PASSED - ES Shell is working correctly!"
    exit 0
} {
    echo "✗ SOME TESTS FAILED:"
    for test = $failed_tests {
        echo "  - $test"
    }
    echo
    echo "Please investigate failed tests before proceeding."
    exit 1
}