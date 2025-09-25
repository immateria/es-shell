. test/test.es

test 'simple math' {
    assert {~ ${%addition 1 2} 3} 'addition 1+2=3'
}