test 'bitshift primitives' {
        assert {~ <={%shl 1 3} 8} 'primitive left'
        assert {~ <={%shr 8 1} 4} 'primitive right'
}

test 'bitshift with variables' {
        X = 1
        S = 3
        Y = 8
        assert {~ <={%shl $X $S} 8} 'primitive left vars'
        assert {~ <={%shr $Y $S} 1} 'primitive right vars'
}
