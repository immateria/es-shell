#!/bin/bash

echo "=== ES Shell Comprehensive Feature Validation ==="
echo "Testing all core functionality with new syntax..."

cd /Users/immateria/ES-SHELL/es-shell

test_pass=0
test_fail=0

run_test() {
    local name="$1"
    local test_code="$2"
    local expected="$3"
    
    echo -n "Testing $name: "
    result=$(./es -c "$test_code" 2>&1)
    if [[ "$result" == "$expected" ]]; then
        echo "PASS"
        ((test_pass++))
    else
        echo "FAIL"
        echo "  Expected: '$expected'"
        echo "  Got:      '$result'"
        ((test_fail++))
    fi
}

echo -e "\n=== Basic Variables and Assignment ==="
run_test "Variable assignment" "x = hello; echo \$x" "hello"
run_test "List assignment" "nums = (1 2 3); echo \$#nums" "3"
run_test "String concatenation" "a = hello; b = world; echo \$a\$b" "helloworld"

echo -e "\n=== Functions ==="
run_test "Function definition" "fn greet name { echo Hello \$name }; greet World" "Hello World"
run_test "Function with multiple args" "fn add x y { echo \$x plus \$y }; add 2 3" "2 plus 3"
run_test "Recursive function" "fn fact n { if {\$n <= 1} {echo 1} {echo \${n times \`{fact \${\$n minus 1}}}}}; fact 3" "6"

echo -e "\n=== Control Flow ==="
run_test "If-then-else (true)" "if {~ hello hello} {echo yes} {echo no}" "yes"
run_test "If-then-else (false)" "if {~ hello world} {echo yes} {echo no}" "no"
run_test "For loop" "result=(); for (i = (a b c)) { result = \$result \$i }; echo \$result" "a b c"

echo -e "\n=== New Expression Syntax ==="
run_test "Basic arithmetic" "echo \${5 plus 3}" "8"
run_test "Variable arithmetic" "a = 10; echo \${a times 2}" "20"
run_test "Mixed expressions" "x = 4; echo \${x plus 5 times 2}" "14"
run_test "Comparison operators" "a = 10; b = 5; echo \${a > b}" "0"
run_test "Complex expressions" "x = 2; y = 3; z = 4; echo \${x times y plus z}" "10"

echo -e "\n=== New Redirection Syntax ==="
run_test "Output redirection" "echo test -> /tmp/es_test_out; cat /tmp/es_test_out; rm /tmp/es_test_out" "test"
run_test "Input redirection" "echo input -> /tmp/es_test_in; cat <- /tmp/es_test_in; rm /tmp/es_test_in" "input"
run_test "Append redirection" "echo line1 -> /tmp/es_test_app; echo line2 ->> /tmp/es_test_app; cat /tmp/es_test_app; rm /tmp/es_test_app" $'line1\nline2'

echo -e "\n=== Pattern Matching ==="
run_test "Glob patterns" "if {~ hello h*} {echo match} {echo nomatch}" "match"
run_test "List patterns" "if {~ b (a b c)} {echo found} {echo notfound}" "found"
run_test "Multiple patterns" "if {~ test (foo bar test)} {echo matched} {echo nomatched}" "matched"

echo -e "\n=== Lists and Iteration ==="
run_test "List construction" "items = (one two three); echo \$#items" "3"
run_test "List access" "nums = (10 20 30); echo \$nums" "10 20 30"
run_test "List flattening" "words = (a b c); echo \$^words" "a b c"

echo -e "\n=== Command Substitution ==="
run_test "Backquote substitution" "result = \`echo hello; echo \$result" "hello"
run_test "Expression evaluation" "result = \${2 times 5}; echo \$result" "10"

echo -e "\n=== Higher-Order Functions ==="
run_test "Simple closure" "fn apply f x { \$f \$x }; fn double n { echo \${n times 2} }; apply double 5" "10"

echo -e "\n=== Let bindings ==="
run_test "Local variables" "let (temp = temporary) { echo \$temp }" "temporary"

echo -e "\n=== Arithmetic Primitives ==="
run_test "Addition primitive" "result = \${3 plus 4}; echo \$result" "7"
run_test "Subtraction primitive" "result = \${10 minus 3}; echo \$result" "7"
run_test "Multiplication primitive" "result = \${6 times 7}; echo \$result" "42"
run_test "Division primitive" "result = \${15 div 3}; echo \$result" "5"

echo -e "\n=== Legacy Compatibility ==="
run_test "Old syntax as text" "echo 'cmd > file'" "cmd > file"
run_test "Comparison operators" "if {5 > 3} {echo greater} {echo notgreater}" "greater"

echo -e "\n=== Summary ==="
total=$((test_pass + test_fail))
echo "Tests completed: $total"
echo "Passed: $test_pass"
echo "Failed: $test_fail"
echo "Success rate: $(( (test_pass * 100) / total ))%"

if [[ $test_fail -eq 0 ]]; then
    echo -e "\n🎉 All tests passed! ES Shell is working perfectly."
    exit 0
else
    echo -e "\n⚠️  Some tests failed. Please review the output above."
    exit 1
fi