test 'math primitives' {
        assert {~ <={%add 1 <={%add 2 3}} 6} 'addition'
        assert {~ <={%sub <={%sub 10 3} 2} 5} 'subtraction'
        assert {~ <={%mul -2 3} -6} 'multiplication'
        assert {~ <={%div 8 2} 4} 'division'
        assert {~ <={%mod 9 4} 1} 'modulus'
}

test 'math with variables' {
        X = 4
        Y = 6
        assert {~ <={%add $X $Y} 10} 'add vars primitive'
        assert {~ <={%sub $Y $X} 2} 'sub vars primitive'
        assert {~ <={%mul $X $Y} 24} 'mul vars primitive'
        assert {~ <={%div $Y $X} 1} 'div vars primitive'
        assert {~ <={%mod $Y $X} 2} 'mod vars primitive'
}

test 'math logs' {
        for ((expr expect) = (
                '%add 1 2' 3
                '%sub 5 2' 3
                '%mul 2 4' 8
                '%div 8 2' 4
                '%mod 9 4' 1
        )) {
                let (out = <={eval $expr}) {
                        echo $expr '->' $out
                        assert {~ $out $expect} $expr
                }
        }
}

