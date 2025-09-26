# ES Shell Redirection Operators Quick Reference

Based on comprehensive testing of the ES shell implementation.

## Working Operators ✅

### Basic Redirection
- `cmd -> file` - Redirect stdout to file (create/overwrite)
- `cmd <- file` - Redirect file to stdin  
- `cmd ->> file` - Append stdout to file

### Advanced Redirection  
- `cmd <-> file` - Bidirectional open-write (read/write mode)
- `cmd <->> file` - Open-append (input redirection with append mode)
- `cmd ->-< file` - Open-create (output with read/write mode)  
- `cmd ->>< file` - Bidirectional append (append with read/write mode)

### Heredoc
- `cmd <--< TAG ... TAG` - Heredoc (multiline input until TAG)

## Non-Working/Issues ❌

### Herestring (Crashes)
- `cmd <--( string` - **CRASHES** (assertion failure in input.c:88)

### Numbered File Descriptors (Not Implemented)  
- `cmd ->[n] file` - **NOT WORKING** (numbered output redirection)

## Usage Examples

```es
# Basic operations
echo "hello" -> /tmp/output.txt
cat <- /tmp/input.txt  
echo "append me" ->> /tmp/log.txt

# Advanced operations
cat <-> /tmp/read_write_file.txt
echo "append input" <->> /tmp/append_file.txt

# Heredoc
cat <--< EOF
Multiple lines
of input  
EOF

# Complex scenarios
{echo "data"} -> /tmp/temp.txt
result = `{cat <- /tmp/temp.txt | wc -w}
```

## Implementation Notes

- Most redirection operators work as documented in `initial.es`
- Herestring syntax needs debugging (input.c assertion failure)
- Numbered file descriptor redirection not yet implemented
- All working operators handle variable expansion correctly
- Complex nesting and piping scenarios work properly

## Test Coverage

See `test/tests/redirection.es` for comprehensive test suite covering all operators with examples and edge cases.