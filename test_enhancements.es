#!/Users/immateria/ES-SHELL/es-shell/es

# Test script for enhanced control structures and list utilities

echo "Testing enhanced ES shell features..."
echo

# Test 1: if-then basic functionality
echo "=== Test 1: if-then (basic) ==="
if-then {true} {echo "✓ if-then with true condition works"}
if-then {false} {echo "✗ This should not print"}
echo

# Test 2: if-then-else
echo "=== Test 2: if-then-else ==="
if-then {true} {echo "✓ True branch works"} else {echo "✗ Should not reach else"}
if-then {false} {echo "✗ Should not print"} else {echo "✓ Else branch works"}
echo

# Test 3: List utilities
echo "=== Test 3: List utilities ==="
fruits = (apple banana cherry)
echo "Original list (with standard echo):"
echo "  Fruits: $fruits"

echo "Enhanced echo-clean:"
echo-clean "  Fruits:" $fruits

echo "Join function:"
joined = <={join ", " $fruits}
echo "  Joined: $joined"

# Test empty list
empty = ()
echo "Empty list test:"
empty_result = <={join "," $empty}
echo "  Empty joined: '$empty_result'"
echo

# Test 4: Test list with single item
echo "=== Test 4: Single item list ==="
single = (onlyitem)
echo-clean "Single item:" $single
single_joined = <={join "," $single}
echo "Single joined: $single_joined"
echo

# Test 5: Traditional if vs enhanced if-then comparison
echo "=== Test 5: Traditional vs enhanced if comparison ==="
x = 5

echo "Traditional if:"
if {~ $x 5} {echo "  ✓ Traditional if works"}

echo "Enhanced if-then:"  
if-then {~ $x 5} {echo "  ✓ Enhanced if-then works"}
echo

echo "All tests completed!"