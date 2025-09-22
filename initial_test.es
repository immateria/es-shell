# Test version with just basic assignments
fn-echo        = $&echo
fn-true        = { result 0 }
fn-access      = $&access
fn-%pathsearch = @ name { access -n $name -1e -xf $path }

# Test variable
home = /
path = /bin /usr/bin

result test completed