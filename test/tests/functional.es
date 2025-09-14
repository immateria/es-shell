# tests/functional.es -- verify higher-order list utilities

fn dup {
        result $1$1
}

fn is-a {
        ~ $1 a
}

fn join {
        result $1$2
}

fn add-X word {
        result X$word
}

fn concat-2 accumulator word {
        result $accumulator$word
}

test 'map duplicates each item' {
        assert {~ <={map dup a b} (aa bb)} 'map collects results'
}

test 'filter selects matching elements' {
        assert {~ <={filter is-a a b a c} (a a)} 'filter keeps a values'
}

test 'reduce concatenates list' {
        assert {~ <={reduce join '' a b c} abc} 'reduce folds list'
}

test 'list-contains? searches lists' {
        assert { list-contains? b (a b c) } 'finds present element'
        assert {! list-contains? z (a b c) } 'fails for missing element'
        assert {! list-contains? a () } 'fails on empty list'
}

test 'for-each? reports predicate failures' {
        TRACE = ()
        assert { for-each? record (a b c) } 'for-each? succeeds when predicate always passes'
        assert {~ $TRACE (a b c)} 'record processed each item'
        TRACE = ()
        fn fail-on-x word { record-fail-on x $word }
        assert { ! for-each? fail-on-x (a x c) } 'for-each? fails when predicate fails'
        assert {~ $TRACE (a x c)} 'all items were processed'
}

test 'reduce-one folds list without seed' {
        assert {~ <={reduce-one concat-2 (a b c)} abc} 'reduce-one concatenates elements'
}

test 'enumerate pairs indexes and values' {
        assert {~ <={enumerate (x y z)} (0 x 1 y 2 z)} 'enumerate outputs index/value pairs'
}

test 'take selects prefix of list' {
        assert {~ <={take 3 (a b c d e)} (a b c)} 'take returns first items'
}

test 'drop skips prefix of list' {
        assert {~ <={drop 2 (a b c d)} (c d)} 'drop removes first items'
}

test 'join-list interleaves separator' {
        assert {~ <={join-list , (a b c)} (a , b , c)} 'join-list inserts delimiter'
}

test 'zip-by-names pairs two lists' {
        LEFT = (a b c)
        RIGHT = (1 2 3 4)
        assert {~ <={zip-by-names LEFT RIGHT} (a 1 b 2 c 3)} 'zip-by-names stops at shorter list'
}
