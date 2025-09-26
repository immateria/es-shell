#!/usr/bin/env es

echo === ES Shell Pattern Operator Test Suite ===
echo

echo --- Phase 1: Basic Pattern Matching Tests ---

# Helper function for testing
fn test_match expr expected {
    if {~ $^expected true} {
        if $expr {
            echo Test passed: '(' $^expr ')'
        } {
            echo Test FAILED: '(' $^expr ')' - expected true, got false
        }
    } {
        if $expr {
            echo Test FAILED: '(' $^expr ')' - expected false, got true  
        } {
            echo Test passed: '(' $^expr ')'
        }
    }
}

# Test 1: Literal exact match
test_match {~ hello hello} true

# Test 2: Literal no match  
test_match {~ hello world} false

# Test 3: Single * wildcard
test_match {~ hello h*} true

# Test 4: Single * no match
test_match {~ hello w*} false

# Test 5: Trailing * match
test_match {~ test.txt test.*} true

# Test 6: Leading * match
test_match {~ test.txt *.txt} true

# Test 7: Middle * match  
test_match {~ test.txt t*.txt} true

# Test 8: Single ? wildcard
test_match {~ hello h?llo} true

# Test 9: Single ? no match
test_match {~ hello h??llo} false

echo
echo --- Phase 2: Pattern Extraction Tests ---

# Test extraction
echo 'Simple extraction:' `{~~ (hello) h*}
echo 'File extension:' `{~~ (file.txt) *.*}
echo 'Complex pattern:' `{~~ (foo.c foo.x bar.h) *.[ch]}

echo
echo All tests completed!