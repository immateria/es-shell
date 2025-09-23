# Test minimal flags
fn-access      = $&access  
path = (/usr/bin /bin)

# Try with just one flag
fn-%pathsearch = @ name { access -n $name $path }