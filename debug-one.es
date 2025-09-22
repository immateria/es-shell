# Debug version of %one function

fn %one-debug {
    echo "Testing %one-debug with args: $*"
    echo "Number of args: $#*"
    
    if {!~ $#* 1} {
        echo "Error condition triggered"
        if {~ $#* 0} {
            echo "No arguments case"
            throw error %one 'null filename in redirection'
        } {
            echo "Too many arguments case"
            throw error %one 'too many files in redirection: ' $*
        }
    }
    echo "Success case"
    result $*
}

echo "Testing %one-debug with no args:"
%one-debug

echo "Testing %one-debug with one arg:"
%one-debug hello

echo "Testing %one-debug with multiple args:"
%one-debug a b c