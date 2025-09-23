# Ultra minimal test
fn-if = $&if

# Try using a primitive directly instead of a function call
if {$&if {result 0} {result 1} {result 0}} {fn-echo = $&echo}