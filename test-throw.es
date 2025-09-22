#!/usr/bin/env es

fn test-throw {
    echo "About to throw error..."
    throw error test 'this is a test error'
    echo "This should not be reached"
}

echo "Calling test-throw..."
test-throw
echo "After test-throw call"