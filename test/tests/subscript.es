# tests/subscript.es -- validate variable subscript parsing

test 'rejects trailing junk in single subscript' {
        foo = (a b c)
        let (exception = ()) {
                catch @ e {exception = $e} {echo $foo(2abc)}
                assert {~ $exception *'bad subscript: 2abc'*}
        }
}

test 'rejects trailing junk in range subscript' {
        foo = (a b c)
        let (exception = ()) {
                catch @ e {exception = $e} {echo $foo(1...3abc)}
                assert {~ $exception *'bad subscript: 1...3abc'*}
        }
}

test 'rejects descending range subscript' {
        foo = (a b c)
        let (exception = ()) {
                catch @ e {exception = $e} {echo $foo(3...2)}
                assert {~ $exception *'bad subscript: 3...2'*}
        }
}
