[![CircleCI](https://circleci.com/gh/wryun/es-shell.svg?style=svg)](https://circleci.com/gh/wryun/es-shell)

Es is an extensible shell. The language was derived from the Plan 9
shell, rc, and was influenced by functional programming languages,
such as Scheme, and the Tcl embeddable programming language. This
implementation is derived from Byron Rakitzis's public domain
implementation of rc.

## New Syntax Features (Version 0.9.3)

This version introduces major syntax enhancements that resolve conflicts
between comparison and redirection operators while adding powerful new
expression evaluation capabilities.

### Redirection Operators

New arrow-based redirection syntax:
```bash
echo "hello" -> output.txt     # Output redirection
cat <- input.txt               # Input redirection  
echo "more" ->> log.txt        # Append redirection
cat <--< EOF                   # Heredoc syntax
Here is some text
EOF
```

### Expression Evaluation

Powerful `${...}` expression evaluation:
```bash
x = ${2 + 3}                   # Arithmetic expressions
result = ${x * 4}              # Variable usage
if {${x} > 5} {                # Expressions in conditions
    echo "x is greater than 5"
}
```

### Comparison & Arithmetic Operators

Now work exclusively as operators (no redirection conflicts):
```bash
if {10 > 5} { echo "greater" }     # Comparison operators
value = ${2 + 3 * 4}               # Arithmetic with precedence (= 14)
check = ${name == "test"}          # String comparison
```

See CHANGELOG.md for complete details.

See the INSTALL file for installation instructions. Once it's running
have a look at the manual page and the docs and examples directories,
in particular Haahr & Rakitzis's paper: ``Es: a shell with higher-order
functions.'' The paper corresponds to a slightly older version of the
shell; see the file ERRATA for changes which affect parts of the paper.

The file initial.es, which is used to build the initial memory state of
the es interpreter, can be read to better understand how pieces of the
shell interact.

The official ftp site (associated with the original authors) is at:

    ftp://ftp.sys.utoronto.ca/pub/es

but all of the relevant information is mirrored in the repository and/or
the website:

    http://www.github.com/wryun/es-shell
    http://wryun.github.io/es-shell

including the change history and the old mailing list archives.

An old version of Paul's .esrc (es startup) file is provided as an
example as esrc.haahr; correctness is not guaranteed. A simple
debugger for es scripts, esdebug, is also included; this is very
untested and should be considered little more than a sketch of a few
ideas.

Copyright
---------

Es is in the public domain. We hold no copyrights or patents on
the source code, and do not place any restrictions on its distribution.
We would appreciate it if any distributions do credit the authors.

Enjoy!

-- Paul Haahr & Byron Rakitzis

Maintenance by:
- Soren Dayton (0.9beta1)
- James Haggerty (post 0.9beta1)
