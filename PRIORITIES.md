# es-shell Development Priorities

## Critical Issues (Fix Immediately)

### 🚨 **Priority 1: Fix Arithmetic Segfaults**
**Status:** CRITICAL BUG - Shell crashes on basic operations
**Impact:** Blocks all usage with symbolic arithmetic

**Problem:** 
- `echo <={3 + 5}` → Segmentation fault
- `echo <={3 - 5}` → Segmentation fault  
- `echo <={3 * 5}` → Segmentation fault
- `echo <={3 plus 5}` → Works fine (8)
- `echo <={%addition 3 5}` → Works fine (8)

**Root Cause:** Bug in symbolic infix arithmetic parsing/evaluation. Word-based infix (`plus`, `minus`) and primitive calls (`%addition`) work correctly, but symbolic operators (`+`, `-`, `*`) cause crashes.

**Investigation Needed:**
- Check `parse.y` arithmetic grammar for symbolic operators
- Review `syntax.c` infix rewriting for symbolic vs word operators
- Test `token.c` tokenization of arithmetic symbols
- Check if symbols are being parsed as commands instead of operators

**Fix Priority:** IMMEDIATE - This makes the shell unusable for arithmetic

---

### 🛡️ **Priority 2: Improve Error Handling**
**Status:** CRITICAL SAFETY
**Impact:** Better user experience and shell stability

**Problems:**
- Invalid syntax should show error messages, not crash
- Need graceful degradation for unsupported operations
- Better error reporting for malformed expressions

**Actions:**
1. Add proper error handling to arithmetic parser
2. Validate expressions before evaluation
3. Add bounds checking in arithmetic operations
4. Improve exception handling in evaluation

---

### 🔧 **Priority 3: Code Stability Review**
**Status:** PREVENTIVE MAINTENANCE
**Impact:** Prevent future crashes

**Actions:**
1. Review other parsing code for similar issues
2. Add more comprehensive test coverage for edge cases
3. Memory safety audit (check for buffer overflows, null pointer dereferences)
4. Validate all recently added arithmetic features

---

## Enhancement Priorities (After Stability)

### 📊 **Priority 4: Expose Dictionary Primitives**
**Status:** HIGH VALUE, LOW RISK
**Impact:** Unlock powerful data structure capabilities

**Rationale:** The infrastructure already exists in `dict.c` - just needs user-facing primitives.

**Implementation:**
```c
// Add to new prim-dict.c:
PRIM(dict_new)      // $&dict-new
PRIM(dict_get)      // $&dict-get key dict  
PRIM(dict_set)      // $&dict-set key value dict
PRIM(dict_has)      // $&dict-has key dict
PRIM(dict_keys)     // $&dict-keys dict
PRIM(dict_values)   // $&dict-values dict
PRIM(dict_delete)   // $&dict-delete key dict
```

**Benefits:** 
- Enables associative arrays for shell scripting
- Leverages existing, tested hash table implementation
- Natural fit with functional programming paradigm

---

### 🌐 **Priority 5: JSON & HTTP Support**
**Status:** HIGH IMPACT FOR MODERN USAGE
**Impact:** Enable modern CLI tool patterns

**JSON Primitives:**
```c
// Add to new prim-json.c:
PRIM(json_parse)    // $&json-parse string → es data structures  
PRIM(json_generate) // $&json-generate data → JSON string
PRIM(json_get)      // $&json-get key json-data
PRIM(json_type)     // $&json-type value → (string|number|boolean|array|object|null)
```

**HTTP Primitives:**
```c  
// Add to new prim-http.c:
PRIM(http_get)      // $&http-get url [headers]
PRIM(http_post)     // $&http-post url data [headers]
PRIM(url_parse)     // $&url-parse url → (scheme host port path query)
```

**Use Cases:**
- API integration: `http-get https://api.github.com/user | json-get login`
- Configuration files: `json-parse <config.json | json-get database.host`
- Data processing pipelines

---

### 🔧 **Priority 6: Enhanced String Operations**
**Status:** BUILDS ON EXISTING STRENGTHS
**Impact:** Complement excellent pattern matching with modern string tools

**Missing Capabilities:**
```c
// Add to enhanced prim-string.c:
PRIM(regex_match)     // Regular expressions (beyond glob patterns)
PRIM(string_trim)     // Remove whitespace: $&string-trim "  hello  " → hello
PRIM(string_replace)  // Replace substrings: $&string-replace "hello world" hello hi
PRIM(base64_encode)   // Encoding: $&base64-encode "hello" → aGVsbG8=
PRIM(base64_decode)   // Decoding: $&base64-decode aGVsbG8= → hello
PRIM(string_split)    // Alternative to $&split with different semantics
```

**Rationale:** es-shell has excellent glob patterns and `~~` extraction, but lacks regex and common string utilities.

---

### 📁 **Priority 7: File & Archive Operations**
**Status:** PRACTICAL UTILITY
**Impact:** Modern file handling capabilities

**File Operations:**
```c
// Add to enhanced prim-file.c:
PRIM(file_hash)     // $&file-hash algorithm file → hash
PRIM(file_copy)     // $&file-copy src dest
PRIM(file_move)     // $&file-move src dest  
PRIM(file_watch)    // $&file-watch file callback (async)
PRIM(file_stat)     // $&file-stat file → (size mtime mode ...)
```

**Archive Support:**
```c
PRIM(tar_list)      // $&tar-list archive.tar
PRIM(tar_extract)   // $&tar-extract archive.tar [dest]
PRIM(zip_create)    // $&zip-create archive.zip files...
```

---

### ⚡ **Priority 8: Async & Concurrency**
**Status:** ADVANCED FEATURE
**Impact:** Modern shell programming patterns

**Parallel Processing:**
```c
// Add to new prim-async.c:
PRIM(parallel_map)  // $&parallel-map function list → results
PRIM(async_exec)    // $&async-exec command → promise-id
PRIM(await)         // $&await promise-id → result
PRIM(promise_all)   // $&promise-all promise-ids → all-results
```

**Use Cases:**
- `parallel-map {|f| file-hash sha256 $f} *.txt` - Hash files in parallel
- `async-exec {long-running-command} | await` - Non-blocking execution

---

### 🧩 **Priority 9: Module System**
**Status:** EXTENSIBILITY FOUNDATION
**Impact:** Enable community contributions without core changes

**Module Primitives:**
```c
// Add to new prim-modules.c:
PRIM(module_load)   // $&module-load path/name
PRIM(module_search) // $&module-search name → path
PRIM(module_list)   // $&module-list → available modules
```

**Standard Library:**
- `json.es` - JSON utilities built on primitives
- `http.es` - HTTP helpers and common patterns  
- `async.es` - Promise-like abstractions
- `testing.es` - Test framework for es scripts

---

## Development Guidelines

### **Stability First**
- All new features must not compromise shell stability
- Comprehensive error handling for all new primitives
- Memory safety and GC integration required
- Test coverage for edge cases mandatory

### **Preserve es Philosophy**  
- Maintain functional programming paradigm
- Keep primitive/function separation clean
- Respect existing `fn-name = $&primitive` pattern
- Build on excellent pattern matching system

### **Implementation Strategy**
1. Fix critical segfaults (Priority 1-3)
2. Add one high-value feature (Priority 4: dictionaries)
3. Validate stability before proceeding
4. Add modern data handling (Priority 5: JSON/HTTP)
5. Continue with remaining priorities based on user feedback

### **Success Metrics**
- ✅ Zero segfaults on basic operations
- ✅ All existing tests pass
- ✅ New features have comprehensive test coverage
- ✅ Performance regression < 5% for existing operations
- ✅ Memory usage growth < 10% for basic shell usage

---

*Last Updated: September 22, 2025*