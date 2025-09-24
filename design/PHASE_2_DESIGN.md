# ES Shell Phase 2 Design Document

## Overview

**Phase 2 Goal**: Transform ES Shell into a modern, user-friendly shell with enhanced UX and contemporary data processing capabilities while preserving its functional programming philosophy.

Phase 1 ✅ **COMPLETED**: Core syntax modernization
- New arrow-based redirection operators (`<-`, `->`, `->>`)
- Expression evaluation syntax (`${...}`)
- Variable expansion in expressions
- Comparison operators in conditionals
- 100% backward compatibility

Phase 2 🎯 **CURRENT**: User experience and modern features
- Enhanced error messages and help system
- Modern data processing (JSON, HTTP, dictionaries)
- Advanced string operations
- Interactive improvements

## Phase 2 Features

### 1. 🚨 **Enhanced Error Messages**
**Priority**: HIGH - Critical for usability

**Current Problems**:
```bash
echo <={1 +}          # → "1: No such file or directory"
unknowncommand        # → "unknowncommand: No such file or directory"
echo {unclosed        # → Generic syntax error
```

**Improved Experience**:
```bash
echo <={1 +}          # → "Syntax error: incomplete arithmetic expression
                      #    Expected: number or operator after '+'
                      #    Try: echo <={1 + 2}"

unknowncommand        # → "Command 'unknowncommand' not found
                      #    Try 'help' for available commands"

echo {unclosed        # → "Syntax error: unclosed brace '{'
                      #    Expected: '}' at end of line"
```

**Implementation**:
- Enhance parser error reporting in `parse.y`
- Add context-aware error messages
- Provide helpful suggestions for common mistakes
- Create error message catalog with examples

### 2. 🆘 **Interactive Help System**
**Priority**: HIGH - Essential for discoverability

**Features**:
```bash
help                  # General help overview
help arithmetic       # Help with arithmetic operations  
help redirection      # Help with new arrow syntax
help functions        # Help with function definition
help primitives       # List all primitives with descriptions
help <primitive>      # Detailed help for specific primitive

discover              # Interactive discovery system
examples              # Show common usage examples
tutorial              # Step-by-step guided tutorial
```

**Implementation**:
- Create comprehensive help content in `initial.es`
- Add help primitive to `prim-etc.c`
- Structured help topics with examples
- Interactive discovery system

### 3. 📦 **Dictionary Primitives**
**Priority**: HIGH - Unlocks powerful data structures

**Rationale**: ES Shell already has a robust hash table implementation in `dict.c`. We just need to expose it as user-facing primitives.

**New Primitives**:
```bash
%dict-new             # Create empty dictionary
%dict-set key value dict  # Set key-value pair, returns new dict
%dict-get key dict    # Get value by key
%dict-has key dict    # Check if key exists
%dict-keys dict       # Get list of all keys
%dict-values dict     # Get list of all values  
%dict-delete key dict # Remove key, returns new dict
%dict-merge dict1 dict2  # Merge two dictionaries
```

**Usage Examples**:
```bash
# Create configuration dictionary
config = <={%dict-new}
config = <={%dict-set host localhost $config}
config = <={%dict-set port 8080 $config}
config = <={%dict-set debug true $config}

# Use configuration
host = <={%dict-get host $config}
echo "Connecting to $host"

# Process all key-value pairs
for (key = <={%dict-keys $config}) {
    value = <={%dict-get $key $config}
    echo "$key = $value"
}
```

**Implementation**:
- Create `src/primitives/prim-dict.c`
- Leverage existing `Dict` type from `dict.c`
- Integrate with garbage collector
- Follow functional programming principles (immutable operations)

### 4. 🌐 **JSON & HTTP Primitives** 
**Priority**: HIGH - Modern data processing needs

**JSON Primitives**:
```bash
%json-parse string    # Parse JSON string to ES data structures
%json-generate data   # Convert ES data to JSON string
%json-get key data    # Extract value from parsed JSON
%json-type value      # Get type: string|number|boolean|array|object|null
```

**HTTP Primitives**:
```bash
%http-get url [headers]     # HTTP GET request
%http-post url data [headers] # HTTP POST request  
%http-put url data [headers]  # HTTP PUT request
%url-parse url        # Parse URL into components
```

**Usage Examples**:
```bash
# API Integration
user = <={%http-get https://api.github.com/users/octocat}
login = <={%json-get login $user}
echo "User: $login"

# Configuration processing
config = <={%json-parse <=cat config.json}
db_host = <={%json-get database.host $config}

# Post data  
response = <={%http-post https://api.example.com/users $user_data}
```

**Implementation**:
- Create `src/primitives/prim-json.c`
- Create `src/primitives/prim-http.c`  
- Use lightweight JSON library (jsmn or similar)
- Use libcurl for HTTP functionality
- Handle errors gracefully

### 5. 🔧 **Enhanced String Operations**
**Priority**: MEDIUM - Complement existing pattern matching

**New String Primitives**:
```bash
%string-trim string   # Remove leading/trailing whitespace
%string-ltrim string  # Remove leading whitespace
%string-rtrim string  # Remove trailing whitespace
%string-replace string old new  # Replace substring
%string-split string delimiter  # Split string (alternative to %split)
%string-join list delimiter     # Join list with delimiter
%string-upper string  # Convert to uppercase
%string-lower string  # Convert to lowercase
%regex-match pattern string     # Regular expression matching
%regex-replace pattern replacement string # Regex replacement
%base64-encode string # Base64 encoding
%base64-decode string # Base64 decoding
%url-encode string    # URL encoding
%url-decode string    # URL decoding
```

**Usage Examples**:
```bash
# Clean up user input
input = "  Hello World  "
clean = <={%string-trim $input}  # "Hello World"

# Text processing
text = "The quick brown fox"
upper = <={%string-upper $text}  # "THE QUICK BROWN FOX"
words = <={%string-split $text " "}  # (The quick brown fox)

# Regular expressions
if {%regex-match "^[0-9]+$" $input} {
    echo "Input is numeric"
}

# Encoding/decoding
encoded = <={%base64-encode "Hello World"}  # SGVsbG8gV29ybGQ=
```

**Implementation**:
- Create `src/primitives/prim-string.c`
- Use POSIX regex library for pattern matching
- Implement base64 encoding/decoding
- URL encoding/decoding support

### 6. 🎨 **Tab Completion Enhancements**
**Priority**: MEDIUM - Improved interactive experience

**Enhanced Completion For**:
- Primitives: `%add<TAB>` → `%addition`
- Variables: `$HO<TAB>` → `$HOME`  
- Functions: `fn-<TAB>` → show available functions
- Files with new redirection: `cat <- fi<TAB>` → file completion
- ES-specific syntax: `<=<TAB>` → `<={}`

**Implementation**:
- Enhance `src/io/input.c` readline integration
- Add ES-aware completion functions
- Context-sensitive completion based on cursor position

### 7. 🎯 **Syntax Validation**
**Priority**: MEDIUM - Real-time help

**Features**:
- Detect unmatched braces: `{echo hello` → suggest closing `}`
- Incomplete expressions: `<={3 +` → suggest completion
- Variable typos: `$HOEM` → suggest `$HOME`
- Invalid syntax: `echo <={` → suggest proper format

**Implementation**:
- Add validation hooks to parser
- Implement suggestion engine
- Integrate with error reporting system

## Implementation Strategy

### Phase 2A: Core Infrastructure (Week 1-2)
1. **Enhanced Error Messages** - Improve parser error reporting
2. **Help System** - Create comprehensive help infrastructure
3. **Dictionary Primitives** - Expose existing hash table functionality

### Phase 2B: Modern Data Processing (Week 3-4)  
4. **JSON Primitives** - JSON parsing and generation
5. **HTTP Primitives** - Basic HTTP client functionality
6. **Enhanced String Operations** - Modern string processing

### Phase 2C: Interactive Improvements (Week 5-6)
7. **Tab Completion** - ES-aware completion
8. **Syntax Validation** - Real-time syntax help
9. **Testing & Polish** - Comprehensive validation

## Success Criteria

### Functional Requirements
- ✅ All new primitives work correctly and handle errors gracefully
- ✅ Help system provides comprehensive, searchable documentation
- ✅ JSON/HTTP operations handle real-world API interactions
- ✅ Dictionary operations support complex data structures
- ✅ Enhanced strings complement existing pattern matching
- ✅ Tab completion improves interactive workflow

### Quality Requirements  
- ✅ 100% backward compatibility maintained
- ✅ No performance regression > 5% for existing operations
- ✅ Memory usage increase < 15% for basic shell usage
- ✅ All features have comprehensive test coverage
- ✅ Error messages are helpful and actionable
- ✅ Documentation is complete and accessible

### User Experience
- ✅ New users can get productive quickly with help system
- ✅ Error messages guide users to correct syntax
- ✅ Modern data processing workflows are intuitive
- ✅ Interactive features feel responsive and helpful
- ✅ ES Shell remains true to functional programming philosophy

## Dependencies

### Build Dependencies
- **libcurl**: HTTP functionality
- **jsmn or cJSON**: Lightweight JSON parsing
- **POSIX regex**: Regular expression support

### Development Tools
- Enhanced testing framework for integration tests
- Performance benchmarking tools
- Memory profiling validation

## Risk Mitigation

### Technical Risks
- **Memory safety**: Thorough testing with valgrind
- **Performance impact**: Benchmark all new primitives
- **Third-party dependencies**: Choose lightweight, stable libraries
- **Backward compatibility**: Extensive regression testing

### User Experience Risks
- **Feature bloat**: Keep features focused and well-integrated
- **Complexity creep**: Maintain ES Shell's elegant simplicity
- **Learning curve**: Ensure help system actually helps users

## Next Steps

1. **Create detailed specifications** for each primitive
2. **Set up build dependencies** (libcurl, JSON library)
3. **Implement Phase 2A features** starting with error messages
4. **Create comprehensive test suite** for each component
5. **Gather user feedback** throughout development process

Phase 2 will transform ES Shell into a modern, approachable shell while preserving its unique functional programming capabilities and maintaining complete backward compatibility.