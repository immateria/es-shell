test 'math primitives' {
        assert {~ <={%addition 1 <={%addition 2 3}} 6} 'addition'
        assert {~ <={%subtraction <={%subtraction 10 3} 2} 5} 'subtraction'
        assert {~ <={%multiplication -2 3} -6} 'multiplication'
        assert {~ <={%division 8 2} 4} 'division'
        assert {~ <={%modulo 9 4} 1} 'modulo'
}

test 'math with variables' {
        X = 4
        Y = 6
        assert {~ <={%addition $X $Y} 10} 'addition vars primitive'
        assert {~ <={%subtraction $Y $X} 2} 'subtraction vars primitive'
        assert {~ <={%multiplication $X $Y} 24} 'multiplication vars primitive'
        assert {~ <={%division $Y $X} 1.5} 'division vars primitive'
        assert {~ <={%modulo $Y $X} 2} 'modulo vars primitive'
}

test 'math logs' {
        for ((expr expect) = (
                '%addition 1 2' 3
                '%subtraction 5 2' 3
                '%multiplication 2 4' 8
                '%division 8 2' 4
                '%modulo 9 4' 1
        )) {
                let (out = <={eval $expr}) {
                        echo $expr '->' $out
                        assert {~ $out $expect} $expr
                }
        }
}

test 'bitwise parsing rejects non-integers' {
        let (exception = ()) {
                catch @ e {exception = $e} {%bitwiseshiftleft '' 1 >[2] /dev/null}
                assert {~ $exception *'value argument must be an integer: '*}
        }

        let (exception = ()) {
                catch @ e {exception = $e} {%bitwiseand 1 not-a-number >[2] /dev/null}
                assert {~ $exception *'each argument must be an integer: not-a-number'*}
        }
}

test 'isint is strict about digits' {
        assert {! {%isint ''}} 'empty strings are not integers'
        assert {! {%isint 'abc'}} 'alphabetic strings are not integers'
}

test 'float parsing rejects overflow' {
        let (exception = ()) {
                catch @ e {exception = $e} {%addition 1e309 >[2] /dev/null}
                assert {~ $exception *'arguments must be numbers'*} 'addition rejects overflow'
        }

        let (exception = ()) {
                catch @ e {exception = $e} {%tofloat 1e309 >[2] /dev/null}
                assert {~ $exception *'argument must be a number'*} 'tofloat rejects overflow'
        }

        assert {! {%isfloat 1e309}} 'overflowing numbers are not floats'
}

