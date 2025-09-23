# Operator Redesign Implementation Guide

## Overview
This document provides detailed implementation guidance for the comprehensive operator redesign in es-shell.

## Token Implementation Priority

### Phase 1: Essential Tokens (MVP)
These must be implemented first to enable basic modern syntax:

```c
// token.h additions
#define EXPR_CALL    300  // ${...} function evaluation
#define LARROW       301  // <- input redirect
#define RARROW       302  // -> output redirect  
#define LARROW_BANG  303  // <-! force input
#define RARROW_BANG  304  // ->! force output
#define HEREDOC_NEW  305  // <--< heredoc start
```

### Phase 2: Comparison Tokens
Free up and repurpose existing symbols:

```c
// Redefine existing (currently used for redirection)
#define LT     '<'   // Now: less than comparison
#define GT     '>'   // Now: greater than comparison
#define LE     306   // <= less than or equal
#define GE     307   // >= greater than or equal
#define EQ     308   // == equality
#define NE     309   // != not equal
#define SEQ    310   // === strict equality
#define SNE    311   // !== strict not equal
```

### Phase 3: Enhanced Operators
```c
#define XOR         312   // ^^ logical XOR
#define NULLISH     313   // ?? nullish coalescing
#define APPROX      314   // ≈ approximately equal
#define SPACESHIP   315   // <=> three-way comparison
#define PREPEND_OP  316   // =+ prepend assignment
#define CONCAT_OP   317   // .= concatenation
#define COND_PIPE   318   // |? conditional pipe
#define PARA_PIPE   319   // ||> parallel pipe
```

## Tokenizer Implementation (token.c)

### 1. Function Call Recognition
Replace the current `<=` CALL token logic:

```c
case '$':
    c = GETC();
    if (c == '{') {
        // Start of ${...} expression
        return EXPR_CALL;
    }
    // Existing $ variable logic
    UNGETC(c);
    return '$';
```

### 2. Arrow Operators
Replace current `<` and `>` cases:

```c
case '<':
    c = GETC();
    if (c == '-') {
        c = GETC();
        if (c == '-') {
            c = GETC();
            if (c == '<') {
                // <--< heredoc start
                return HEREDOC_NEW;
            }
            UNGETC(c);
            // Could be other <-- pattern
        } else if (c == '!') {
            // <-! force input
            return LARROW_BANG;
        } else if (c == '>') {
            // <-> bidirectional
            c = GETC();
            if (c == '!') {
                return BIDI_BANG;
            }
            UNGETC(c);
            return BIDI;
        } else {
            // <- input redirect
            UNGETC(c);
            return LARROW;
        }
    } else if (c == '=') {
        // <= comparison or here string
        c = GETC();
        if (isspace(c) || c == EOF) {
            // It's a here string (replacing <<<)
            UNGETC(c);
            return HERE_STRING;
        } else {
            // <= comparison
            UNGETC(c);
            return LE;
        }
    } else {
        // < comparison operator
        UNGETC(c);
        return LT;
    }

case '>':
    c = GETC();
    if (c == '=') {
        // >= comparison
        return GE;
    } else {
        // > comparison (need to check for -> patterns in separate logic)
        UNGETC(c);
        // Check context to determine if comparison or start of ->
        if (check_arrow_context()) {
            return parse_output_arrow();
        }
        return GT;
    }

case '-':
    c = GETC();
    if (c == '>') {
        c = GETC();
        if (c == '>') {
            // ->> append
            return RARROW_APPEND;
        } else if (c == '!') {
            // ->! force output
            return RARROW_BANG;
        } else {
            // -> output redirect
            UNGETC(c);
            return RARROW;
        }
    }
    // Regular minus operator
    UNGETC(c);
    return '-';
```

### 3. Unicode Support
Add UTF-8 aware tokenization:

```c
static int read_utf8_char(void) {
    int c = GETC();
    if ((c & 0x80) == 0) {
        // ASCII
        return c;
    }
    
    // Multi-byte UTF-8
    unsigned char buf[4];
    int len;
    buf[0] = c;
    
    if ((c & 0xE0) == 0xC0) len = 2;
    else if ((c & 0xF0) == 0xE0) len = 3;
    else if ((c & 0xF8) == 0xF0) len = 4;
    else return c; // Invalid UTF-8
    
    for (int i = 1; i < len; i++) {
        c = GETC();
        if ((c & 0xC0) != 0x80) {
            // Invalid continuation
            UNGETC(c);
            return buf[0];
        }
        buf[i] = c;
    }
    
    // Check for known Unicode operators
    if (len == 3) {
        if (memcmp(buf, "\xE2\x89\xA4", 3) == 0) return LE_UNICODE;  // ≤
        if (memcmp(buf, "\xE2\x89\xA5", 3) == 0) return GE_UNICODE;  // ≥
        if (memcmp(buf, "\xE2\x89\xA0", 3) == 0) return NE_UNICODE;  // ≠
        if (memcmp(buf, "\xE2\x89\x88", 3) == 0) return APPROX;      // ≈
        // Add more as needed
    }
    
    // Not a recognized operator, treat as word character
    for (int i = len - 1; i >= 0; i--) {
        UNGETC(buf[i]);
    }
    return buf[0];
}
```

## Parser Grammar Updates (parse.y)

### 1. New Tokens
```yacc
%token EXPR_CALL LARROW RARROW LARROW_BANG RARROW_BANG
%token HEREDOC_NEW HERE_STRING BIDI BIDI_BANG
%token LE GE EQ NE SEQ SNE APPROX SPACESHIP
%token XOR NULLISH COND_PIPE PARA_PIPE
%token PREPEND_OP CONCAT_OP

// Unicode aliases
%token LE_UNICODE GE_UNICODE NE_UNICODE
```

### 2. Updated Precedence
```yacc
%right '=' PREPEND_OP CONCAT_OP   // Assignment operators
%left OROR '||'
%left XOR '^^'
%left ANDAND '&&'
%nonassoc EQ NE SEQ SNE
%nonassoc LT GT LE GE APPROX
%left '|' COND_PIPE PARA_PIPE
%left '^'
%left '&'
%left '+' '-'
%left '*' '/' '%'
%right '**' POW
%right '!' '~' UNARY
```

### 3. Grammar Rules
```yacc
comword : EXPR_CALL body '}'     { $$ = mk(nCall, $2); }
        | // existing rules...

redir   : LARROW word             { $$ = mkredir("%open", $2); }
        | RARROW word             { $$ = mkredir("%create", $2); }
        | LARROW_BANG word        { $$ = mkredir("%open-force", $2); }
        | RARROW_BANG word        { $$ = mkredir("%create-force", $2); }
        | HEREDOC_NEW word        { $$ = mkredir("%heredoc", $2); }
        | HERE_STRING word        { $$ = mkredir("%here", $2); }
        | // existing rules...

comparison : expr LT expr         { $$ = mkcomp("less", $1, $3); }
           | expr GT expr         { $$ = mkcomp("greater", $1, $3); }
           | expr LE expr         { $$ = mkcomp("lessequal", $1, $3); }
           | expr GE expr         { $$ = mkcomp("greaterequal", $1, $3); }
           | expr EQ expr         { $$ = mkcomp("equal", $1, $3); }
           | expr NE expr         { $$ = mkcomp("notequal", $1, $3); }
           | expr SEQ expr        { $$ = mkcomp("strictequal", $1, $3); }
           | expr SNE expr        { $$ = mkcomp("strictnotequal", $1, $3); }
           | expr APPROX expr     { $$ = mkcomp("approx", $1, $3); }
           | expr SPACESHIP expr  { $$ = mkcomp("threeway", $1, $3); }
```

## Migration Tool (es-migrate)

Create a shell script that converts old syntax to new:

```bash
#!/usr/bin/env es

fn-migrate = @ file {
    content = `{cat $file}
    
    # Function calls: <={ ... } -> ${ ... }
    content = `{echo $content | sed 's/<=\{/\$\{/g'}
    
    # Input redirection: < file -> <- file
    content = `{echo $content | sed 's/^\([^<]*\)< \([^ ]*\)/\1<- \2/g'}
    
    # Output redirection: > file -> -> file
    content = `{echo $content | sed 's/^\([^>]*\)> \([^ ]*\)/\1-> \2/g'}
    
    # Append: >> file -> ->> file
    content = `{echo $content | sed 's/>>/->>/g'}
    
    # Here doc: <<EOF -> <--<EOF
    content = `{echo $content | sed 's/<<\([A-Z]*\)/<--<\1/g'}
    
    # Here string: <<< -> <=
    content = `{echo $content | sed 's/<<</<=g'}
    
    echo $content
}
```

## Testing Strategy

### 1. Unit Tests for Tokenizer
```c
void test_new_operators() {
    assert(tokenize("${expr}") == EXPR_CALL);
    assert(tokenize("<-") == LARROW);
    assert(tokenize("->") == RARROW);
    assert(tokenize(">=") == GE);
    assert(tokenize("≥") == GE_UNICODE);
    // etc...
}
```

### 2. Parser Tests
```es
# test-operators.es
test 'function evaluation' {
    assert {~ ${ 2 + 3 } 5} 'basic ${} evaluation'
}

test 'arrow redirection' {
    echo "test" -> temp.txt
    content = `{cat <- temp.txt}
    assert {~ $content "test"} 'arrow I/O works'
}

test 'comparison operators' {
    assert {10 > 5} 'greater than'
    assert {5 <= 5} 'less than or equal'
    assert {"foo" == "foo"} 'string equality'
}
```

### 3. Migration Tests
Verify that migrated code produces identical results:

```bash
# Run old and new versions, compare outputs
./es-old script-old.es > old.out
./es-migrate script-old.es > script-new.es
./es-new script-new.es > new.out
diff old.out new.out
```

## Compatibility Mode Implementation

Add runtime flag checking:

```c
// In main() or initialization
int classic_operators = 0;
if (getenv("ES_CLASSIC_OPS") || has_flag("--classic-operators")) {
    classic_operators = 1;
}

// In tokenizer
if (classic_operators) {
    // Use old tokenization rules
} else {
    // Use new tokenization rules
}
```

## Documentation Updates

### Man Page Sections
1. OPERATORS - Complete operator reference
2. MIGRATION - Guide for updating scripts
3. COMPATIBILITY - Using classic mode
4. UNICODE - UTF-8 operator support

### Examples to Add
- Modern expression evaluation
- Arrow-based I/O
- Advanced assignment operators
- Pipeline variants
- Unicode mathematics

## Performance Considerations

1. **Token Lookahead Buffer**: Increase for multi-char operators
2. **Unicode Caching**: Cache UTF-8 operator lookups
3. **Operator Dispatch Table**: Optimize for common operators
4. **Compatibility Check**: Make it a compile-time option when possible

## Timeline Estimate

- **Week 1-2**: Core tokenizer changes, basic arrow operators
- **Week 3-4**: Parser grammar updates, ${} evaluation
- **Week 5-6**: Comparison operators, Unicode support
- **Week 7-8**: Advanced operators, assignment enhancements
- **Week 9-10**: Migration tool, compatibility mode
- **Week 11-12**: Testing, documentation, optimization

## Success Metrics

1. All existing tests pass in compatibility mode
2. Migration tool converts 95%+ of common patterns
3. No performance regression in benchmarks
4. Unicode operators work in UTF-8 locales
5. Clear error messages for syntax errors