# Test with spaced arguments
fn-access      = $&access  
path = (/usr/bin /bin)

# Try original style but check each piece
fn-%pathsearch = @ name { access -n $name (-1e) (-xf) $path }