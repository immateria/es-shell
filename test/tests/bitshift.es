test 'bitshift primitives' {
        assert {~ <={%bitwiseshiftleft 1 3} 8} 'primitive left'
        assert {~ <={%bitwiseshiftright 8 1} 4} 'primitive right'
}

test 'bitshift with variables' {
        X = 1
        S = 3
        Y = 8
        assert {~ <={%bitwiseshiftleft $X $S} 8} 'primitive left vars'
        assert {~ <={%bitwiseshiftright $Y $S} 1} 'primitive right vars'
}
