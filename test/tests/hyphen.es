test 'hyphenated function names' {
	fn-foo-bar = { result ok }
	assert {~ <={foo-bar} ok} 'invoke hyphenated name'
}
