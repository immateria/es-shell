# Test pathsearch with access
fn-access      = $&access
path = (/usr/bin /bin)
fn-%pathsearch = @ name { access -n $name -1e -xf $path }