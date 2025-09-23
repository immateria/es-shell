# initial.es -- set up initial interpreter state with early %pathsearch

# Define critical functions first to enable bootstrap
fn-if          = $&if
fn-echo        = $&echo  
fn-result      = $&result
fn-access      = $&access
fn-true        = { result 0 }
fn-false       = { result 1 }

# Define %pathsearch VERY early to enable command resolution
fn-%pathsearch = @ name { access -n $name -1e -xf $path }

# Now test the problematic construct that was failing before
if {~ <=$&primitives limit} {fn-limit = $&limit}
if {~ <=$&primitives time}  {fn-time  = $&time}