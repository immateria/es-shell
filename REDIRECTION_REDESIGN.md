# Comprehensive Operator Redesign

## Objective
Create a modern, intuitive operator system by:
1. Freeing up `<`, `>`, `<=`, `>=`, `==`, `!=` for use as comparison operators
2. Implementing arrow-based redirection syntax that's more intuitive
3. Introducing `${...}` for function evaluation (replacing `<={...}`)
4. Adding Unicode operator alternatives for mathematical clarity
5. Enhancing assignment and pipeline operators

## Current Patterns to Replace

| Pattern | Command | Description |
|---------|---------|-------------|
| `<` | `%open` | Input redirection (read from file) |
| `<>` | `%open-write` | Read-write mode |
| `<>>` | `%open-append` | Read-append mode |
| `<<` | `%heredoc` | Heredoc |
| `<<<` | `%here` | Here string |
| `<=` | `CALL` token | Function call (this is why `<={...}` works!) |
| `>` | `%create` | Output redirection (write to file) |
| `>>` | `%append` | Append to file |
| `><` | `%open-create` | Create for read-write |
| `>><` | `%open-append` | Append for read-write |

## New Operator System

### 1. Function Evaluation (replacing `<={...}`)
| New Pattern | Token | Old Pattern | Description |
|-------------|-------|-------------|-------------|
| `${...}` | `CALL` | `<={...}` | Function/expression evaluation |

**Rationale:** `${...}` mirrors variable expansion patterns and clearly indicates "evaluate this expression" - familiar to modern shell and JavaScript developers.

### 2. Redirection Operators

#### Input Operations
| New Pattern | Command | Old Pattern | Description |
|-------------|---------|-------------|-------------|
| `<-` | `%open` | `<` | stdin from file |
| `<-!` | `%open-force` | N/A | Force overwrite existing stdin |
| `<--<EOF` | `%heredoc` | `<<EOF` | Heredoc |
| `<--<'EOF'` | `%heredoc-quoted` | `<<'EOF'` | Quoted heredoc (no expansion) |
| `<<--<EOF` | `%heredoc-strip` | `<<-EOF` | Stripped heredoc (remove leading whitespace) |
| `<=` | `%here` | `<<<` | Here string |
| `<->` | `%open-write` | `<>` | Read-write mode |
| `<->!` | `%open-write-force` | N/A | Force read-write |

#### Output Operations  
| New Pattern | Command | Old Pattern | Description |
|-------------|---------|-------------|-------------|
| `->` | `%create` | `>` | stdout to file |
| `->!` | `%create-force` | `>\|` | Force overwrite (bypass noclobber) |
| `->>` | `%append` | `>>` | Append to file |
| `=->` | `%tee` | N/A | Tee operation (output + passthrough) |
| `!->` | `%create-stderr` | `2>` | stderr to file |
| `&->` | `%create-both` | `&>` | Both stdout and stderr to file |

### 3. Comparison Operators

With redirection symbols freed, we can implement standard comparison operators with Unicode alternatives:

| ASCII | Unicode | Token | Primitive Call | Description |
|-------|---------|-------|----------------|-------------|
| `<` | - | `LT` | `$&less` | Less than |
| `>` | - | `GT` | `$&greater` | Greater than |
| `<=` | `≤` | `LE` | `$&less-equal` | Less than or equal |
| `>=` | `≥` | `GE` | `$&greater-equal` | Greater than or equal |
| `==` | `≡` | `EQ` | `$&equal` | Equal |
| `!=` | `≠` | `NE` | `$&not-equal` | Not equal |
| `===` | `≣` | `SEQ` | `$&strict-equal` | Strict equality (type-aware) |
| `!==` | `≢` | `SNE` | `$&strict-not-equal` | Strict inequality |
| `<=>` | - | `CMP` | `$&compare` | Three-way comparison |
| `≈` | - | `APX` | `$&approx` | Approximately equal (floats) |
| `≉` | - | `NAPX` | `$&not-approx` | Not approximately equal |

### 4. Logical Operators

| ASCII | Word | Unicode | Description |
|-------|------|---------|-------------|
| `&&` | `and` | `∧` | Logical AND |
| `\|\|` | `or` | `∨` | Logical OR |
| `!` | `not` | `¬` | Logical NOT |
| `^^` | `xor` | `⊕` | Logical XOR |

### 5. Assignment Operators

| Operator | Description | Example |
|----------|-------------|----------|
| `=` | Standard assignment | `x = 5` |
| `:=` | Immediate evaluation | `result := ${compute()}` |
| `?=` | Assign if unset | `var ?= "default"` |
| `??=` | Assign if null/unset | `var ??= "default"` |
| `\|\|=` | Assign if falsy | `var \|\|= "default"` |
| `&&=` | Assign if truthy | `flag &&= validate()` |
| `+=` | Append/add | `count += 1` |
| `=+` | Prepend | `path =+ "/usr/local/bin:"` |
| `-=` | Remove/subtract | `count -= 1` |
| `*=` | Multiply and assign | `total *= factor` |
| `/=` | Divide and assign | `avg /= count` |
| `.=` | Concatenate | `str .= " suffix"` |
| `//=` | Regex replace and assign | `text //= 's/old/new/g'` |

### 6. Pipeline Operators

| Operator | Description | Example |
|----------|-------------|----------|
| `\|` | Standard pipe | `ls \| grep txt` |
| `\|>` | Error pipe (stderr only) | `cmd \|> errors.log` |
| `\|&` | Pipe both stdout and stderr | `build \|& tee log.txt` |
| `\|?` | Conditional pipe | `find . \|? xargs rm` |
| `\|\|>` | Parallel pipe | `data \|\|> process1 \|\|> process2` |
| `\|tee` | Visible pipe | `download \|tee progress` |

### 7. Pattern Matching Operators

| Operator | Description | Example |
|----------|-------------|----------|
| `=~` | Regex match | `if {$text =~ /pattern/}` |
| `!~` | Negative regex match | `if {$text !~ /error/}` |
| `~=` | Glob match | `if {$file ~= "*.txt"}` |
| `!~=` | Negative glob match | `if {$file !~= "*.tmp"}` |
| `∈` | Element in collection | `if {$item ∈ $list}` |
| `∉` | Not in collection | `if {$item ∉ $list}` |

## Implementation Steps

### Phase 1: Core Token Changes

1. **Update parse.y tokens:**
   ```c
   %token LT GT LE GE EQ NE SEQ SNE CMP  // Comparison tokens
   %token APX NAPX                       // Approximate equality
   %token EXPR_CALL                      // New function call token for ${...}
   %token LARROW RARROW                  // <- and -> base tokens
   %token LARROW_FORCE RARROW_FORCE      // <-! and ->!
   %token HEREDOC_START                  // <--< token
   %token PARALLEL_PIPE COND_PIPE        // ||> and |?
   ```

2. **Update tokenizer in token.c:**
   - Implement `$` followed by `{` as EXPR_CALL token
   - Replace `<` case logic with arrow patterns (`<-`, `<->`, `<--<`, etc.)
   - Replace `>` case logic with arrow patterns (`->`, `->>`, `=->`, etc.)
   - Add Unicode operator detection (with UTF-8 awareness)
   - Change `<=` from CALL to LE comparison token
   - Add `^^` for XOR, `??=` for nullish coalescing

3. **Add Unicode support detection:**
   ```c
   static int unicode_level = UNICODE_NONE;
   
   void detect_unicode_support() {
       char *lang = getenv("LANG");
       char *es_unicode = getenv("ES_UNICODE_LEVEL");
       
       if (es_unicode) {
           if (streq(es_unicode, "full")) unicode_level = UNICODE_FULL;
           else if (streq(es_unicode, "basic")) unicode_level = UNICODE_BASIC;
       } else if (lang && strstr(lang, "UTF-8")) {
           unicode_level = UNICODE_BASIC;
       }
   }
   ```

### Phase 2: Grammar Updates

4. **Update parser grammar:**
   - Change function call syntax: `comword : EXPR_CALL body '}'`
   - Add comparison operators to expression rules with proper precedence
   - Update redirection rules for arrow syntax
   - Add new assignment operators (`?=`, `??=`, `||=`, `+=`, `=+`, etc.)
   - Implement pipeline operator variants

5. **Precedence adjustments in parse.y:**
   ```yacc
   %left '||' OROR
   %left '^^' XOR  
   %left '&&' ANDAND
   %left '|'
   %left '^'
   %left '&'
   %left EQ NE SEQ SNE
   %left LT GT LE GE
   %left APX NAPX
   %left '<<' '>>'
   %left '+' '-'
   %left '*' '/' '%'
   %right '**' POW
   %right '!' '~' UNARY
   ```

### Phase 3: Runtime Support

6. **Create new primitives:**
   - `prim_strict_equal` for `===` operator
   - `prim_approx_equal` for `≈` operator
   - `prim_three_way_compare` for `<=>` operator
   - `prim_prepend` for `=+` operator
   - Enhanced assignment primitives for `??=`, `||=`, etc.

7. **Update syntax.c:**
   - Extend `classifyinfix` to recognize new operators
   - Add Unicode operator mappings
   - Implement operator precedence for new operators

### Phase 4: Shell Environment Updates

8. **Update initial.es:**
   - Convert all `<={...}` to `${...}`
   - Update redirection syntax in bootstrap code
   - Add operator feature detection functions
   - Implement compatibility wrappers if needed

9. **Update test suite:**
   - Convert test files to new syntax
   - Add tests for all new operators
   - Test Unicode operators with different locale settings
   - Verify precedence rules

## Migration Strategy

### Phase 1: Dual Syntax Support (Compatibility Mode)
- Implement new operators alongside old ones
- Add deprecation warnings for old syntax (suppressible)
- Allow both `<={...}` and `${...}` for function calls
- Provide `--classic-operators` flag for pure backward compatibility
- Comprehensive testing of both syntaxes

### Phase 2: Transition Period
- Convert `initial.es` incrementally with fallback detection
- Update all test files to new syntax
- Provide automated migration script:
  ```bash
  es-migrate --convert-operators script.es
  ```
- Update documentation with migration guide
- Add syntax highlighting support for new operators

### Phase 3: Modern Mode Default
- Make new syntax the default
- Old syntax requires explicit `--classic-operators` flag
- `<=` becomes purely a comparison operator
- Full Unicode operator support when locale permits

### Phase 4: Legacy Removal (Optional, Version 2.0)
- Remove old operator support entirely
- Simplify parser by removing compatibility code
- Full commitment to modern syntax

## Example Conversions

### Basic Redirection
```bash
# Old syntax
echo "hello" > output.txt
cat < input.txt
command >> log.txt
command 2> errors.txt

# New syntax  
echo "hello" -> output.txt
cat <- input.txt
command ->> log.txt
command !-> errors.txt
```

### Function Evaluation
```bash
# Old syntax
result = <={ $&addition 3 5 }
sum = <={ 3 plus 4 }
value = <={ $x * $y + $z }

# New syntax
result = ${ $&addition 3 5 }
sum = ${ 3 plus 4 }
value = ${ $x * $y + $z }
```

### Heredocs
```bash
# Old syntax
cat <<EOF
Multiline
text here
EOF

# New syntax
cat <--<EOF
Multiline
text here
EOF

# Quoted heredoc (no variable expansion)
cat <--<'TEMPLATE'
Literal $HOME and $USER
TEMPLATE

# Stripped heredoc (removes leading tabs)
cat <<--<EOF
	This leading tab is removed
	So is this one
EOF
```

### Comparisons with Unicode
```bash
# Standard operators
cond { $score > 80 } then { echo "Good" }
cond { $x <= $y } then { echo "X is smaller or equal" }
cond { $name == "John" } then { echo "Hello John" }

# Unicode alternatives
cond { $score ≥ 90 } then { echo "Excellent" }
cond { $value ≠ 0 } then { echo "Non-zero" }
cond { $pi ≈ 3.14159 } then { echo "Close enough" }
cond { $user ∈ $admins } then { echo "Admin user" }
```

### Advanced Operators
```bash
# Nullish coalescing assignment
config_value ??= "default_setting"

# Logical assignment
verbose ||= ${detect_debug_mode()}

# Prepend operator
PATH =+ "/usr/local/bin:"

# Parallel pipes
data_stream ||> analyzer1 ||> analyzer2 ||> aggregator

# Conditional pipe
find . -name "*.tmp" |? xargs rm -f

# Tee operation
download_file =-> output.bin | progress_monitor

# XOR operations
if { $opt1 ^^ $opt2 } { echo "Exactly one option set" }
if { $flag1 xor $flag2 } { echo "Mutually exclusive" }
```

## Benefits

1. **Modern and Intuitive:** Aligns with JavaScript, Python, and other modern languages
2. **Clear Semantics:** Arrow operators (`->`, `<-`) visually indicate direction of data flow
3. **Familiar Function Syntax:** `${...}` mirrors variable expansion, making evaluation explicit
4. **Unicode Support:** Mathematical operators (≤, ≥, ≠, ≈) improve readability for mathematical expressions
5. **Powerful Assignments:** Operators like `??=` and `||=` reduce boilerplate code
6. **Enhanced Pipelines:** Conditional and parallel pipes add new capabilities
7. **Backwards Compatible Migration:** Phased approach allows gradual adoption

## Implementation Challenges

1. **Parser Complexity:** Need to carefully handle operator precedence and conflicts
2. **Unicode Detection:** Must gracefully handle non-UTF-8 environments
3. **Migration Tooling:** Need robust scripts to convert existing code
4. **Documentation:** Extensive docs needed for new operators
5. **Testing Coverage:** Every operator combination needs thorough testing

## Performance Considerations

- **Token Lookahead:** Multi-character operators like `<--<` require more lookahead
- **Unicode Processing:** UTF-8 parsing adds minor overhead
- **Operator Dispatch:** More operators mean larger dispatch tables
- **Compatibility Checks:** Dual-mode support has runtime overhead (removable in v2.0)

## Conclusion

This comprehensive operator redesign transforms es-shell into a modern, expressive shell that feels natural to contemporary developers while maintaining the power of traditional shell scripting. The arrow-based redirection (`->`, `<-`) provides intuitive I/O operations, `${...}` makes expression evaluation explicit, and the addition of Unicode operators with ASCII fallbacks ensures both elegance and compatibility.

The phased migration strategy ensures existing scripts can continue to work while the ecosystem transitions to the new, more powerful syntax. This positions es-shell as a forward-thinking shell that bridges the gap between traditional Unix shells and modern programming languages.
