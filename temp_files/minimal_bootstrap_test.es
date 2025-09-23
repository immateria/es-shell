# Minimal bootstrap test
fn-if = $&if
fn-echo = $&echo
fn-result = $&result
fn-true = { result 0 }
fn-false = { result 1 }

# Test simple if
if {true} {echo "if works"}

# Test the problematic construct
if {~ <=$&primitives limit} {echo "limit primitive found"}