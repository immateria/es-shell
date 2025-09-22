fn-whatis = @ {
    let (result = ) {
        for (i = $*) {
            catch @ e from message {
                if {!~ $e error} {
                    throw $e $from $message
                }
                echo >[1=2] $message
                result = $result 1
            } {
                echo <={%whatis $i}
                result = $result 0
            }
        }
        result $result
    }
}