# Minimal initial test
fn-%pathsearch = @ file path {
    for (dir = $path) {
        full = $dir/$file
        if {access -f $full} { result $full ; return }
    }
    result $file
}

echo hello