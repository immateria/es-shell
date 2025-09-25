#!/usr/local/bin/es

echo Testing ~ operator
if {~ 3 3} { echo '~ operator works' } { echo '~ operator fails' }

echo Testing command result
if {true} { echo 'true command works' } { echo 'true command fails' }

echo Testing assertion function
fn-assert = @ cmd message {
    echo 'Running assertion:' $^cmd
    if $cmd { 
        echo PASS: $^message
    } { 
        echo FAIL: $^message
    }
}

assert {~ 3 3} 'simple match'