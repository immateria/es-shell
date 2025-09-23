# initial.es -- set up initial interpreter state with early bootstrap

# EARLY BOOTSTRAP: Define critical functions first
fn-if          = $&if
fn-echo        = $&echo  
fn-result      = $&result
fn-access      = $&access
fn-true        = { result 0 }
fn-false       = { result 1 }
fn-throw       = $&throw

# Initialize basic PATH for command resolution
path = (/usr/bin /bin /usr/local/bin /opt/homebrew/bin)

# Define %pathsearch early to enable command resolution during bootstrap
fn-%pathsearch = @ name { access -n $name -1 -e -x -f $path }

# Define other essential % functions used during bootstrap
fn-%home       = $&home

# Now we can safely use if statements and command resolution
if {~ <=$&primitives limit} {fn-limit = $&limit}
if {~ <=$&primitives time}  {fn-time  = $&time}

# Continue with basic function definitions
fn-.           = $&dot
fn-break       = $&break
fn-catch       = $&catch
fn-exec        = $&exec
fn-forever     = $&forever
fn-fork        = $&fork
fn-newpgrp     = $&newpgrp
fn-umask       = $&umask
fn-wait        = $&wait
fn-%read       = $&read