# Test minimal file with just pathsearch and echo
fn %pathsearch name { access -n $name -1e -xf $path }
echo hello