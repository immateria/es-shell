# Test different argument styles
fn-access      = $&access
path = (/usr/bin /bin)

# Try with explicit list structure
fn-%pathsearch = @ name { access (-n $name -1e -xf) $path }