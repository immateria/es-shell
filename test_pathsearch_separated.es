# Test with separated flags
fn-access      = $&access  
path = (/usr/bin /bin)

# Try separating -1 and -e
fn-%pathsearch = @ name { access -n $name -1 -e -x -f $path }