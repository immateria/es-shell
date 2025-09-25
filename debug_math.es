#!/usr/local/bin/es

# Debug math test

. test/test.es

test 'debug math' {
    echo 'Testing addition: ${%addition 1 2}'
    result = ${%addition 1 2}
    echo 'Result:' $result
    
    echo 'Testing assertion'
    assert {~ $result 3} 'simple addition'
}