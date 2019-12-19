# Ri

Tiny statically-typed (embeddable) language.

# Motivation

- Language to replace Lua where more stable performance and control is needed.
- Language to replace C with language of similar control, but better possibility of code organization (packages or namespaces).
- Language that maps APIs and paradigms to the script with zero friction.

## Runt

Project is not being made in isolation, it is made as a direct requirement of [Runt](https://github.com/martincohen/Runt) project. _Runt_ needs this for it's configuration, command customization and extensions.

# Inspiration

- [Go](https://golang.org/ref/spec) -- Love syntax and a feature set of Go (minus garbage collection)
- [WebAssembly](https://webassembly.github.io/spec/core/index.html) -- Love most of the design, VM takes big chunk of notes from this.
- [Quake3 VM](https://www.icculus.org/~phaethon/q3mc/q3vm_specs.html) -- Love idea of compiling VM to x64 at runtime, VM/x64 takes notes from this.
- [Bitwise's Ion](https://github.com/pervognsen/bitwise/tree/master/ion) -- Ideas and knowledge stolen with love.

# Features

Implementation:

- Embeddable.
- STB-style libaries.
- Minimal binary size.
- No cstdlib dependency.
- No LLVM dependency.

Language:

- Simple Go-inspired syntax.
- Type inference.
- No built-in memory management.
- Packages.
- Most of what C does.
- Bring your own library code.
- Compile time execution.

# Organization

Project is planned to be split to STB-style modules:

- `ri.h`
    - Library
    - Lexing, Parsing, Resolution, Validation.
    - Take string, output AST or error.
- `ri-to-vm.h`
    - Library
    - Take _RiAST_, output _RiVM_ module blob.
- `ri-vm.h`
    - Library
    - Take _RiVM_ blob, run it.

Standalone programs:

- `ri-to-c.c`
    - Program
    - Take _RiAST_, output C file(s).
    - Run compiler.
    - Run executable.

# To do

The work is split to phases:

- **Phase 1** AST: Basics (current)
    - Lexer, parser and resolver basics
    - All statements
    - Simple scalar types
    - Draft VM compilation
- **Phase 2** AST: Compound types
    - Enum
    - Pointer
    - Struct
    - Union
    - Initializers
- **Phase 3** AST: Slices
    - C arrays with count
    - Packages
- **Phase 4** VM: Compilation and Execution
    - Compiling to VM
    - Integration with _Runt_ using simple AST interpreter for initializers.
- **Later**:
    - Compiling VM to x64
    - Compiling to C

The list will grow as I go.

- [x] Built-in types
    - [x] `int8`
    - [x] `uint8`
    - [x] `int16`
    - [x] `uint16`
    - [x] `int32`
    - [x] `uint32`
    - [x] `int64`
    - [x] `uint64`
    - [x] `float32`
    - [x] `float64`
    - [x] `bool`
    - [ ] Complex type(s)
        - `complex64`
        - `complex128`
    - [ ] SSE type(s)
    - [ ] Pointers
        - [ ] Function pointers
    - [ ] Struct
    - [ ] Union
    - [ ] Enum
    - [ ] Slices
        - `[]type`
- [ ] Constants
    - [ ] Untyped constant literal
        - [x] Integer
        - [x] Real
        - [x] Boolean
        - [ ] String
        - [ ] Initializers
        - [ ] Nil
    - [ ] Typed named constants
    - [ ] Untyped named constants
- [ ] Comments
    - [x] Line comments
    - [ ] Block comments
- [x] Function declaration
    - [x] `function <name> (<inputs>) (<outputs>) { ... }`
    - [ ] `function <name> (<inputs>) <type-spec>`
    - [ ] `function <name> (<inputs>)`
    - [ ] Convenience syntax for function arguments of same type (`a, b int32`)
- [ ] Function prototype declaration
    - [x] `function <name> (<inputs>) (<outputs>)`
- [ ] Function type
    - [ ] `function (<inputs>) (<outputs>)`
    - [ ] `function (<inputs>) <type-spec>`
    - [ ] `function (<inputs>)`
- [x] Call
    - [x] `<name>(<arguments>)`
    - [ ] Type checking.
- [x] If statement
    - [x] `if <pre-st>; <condition> { ... }`
    - [x] `if <condition> { ... }`
    - [x] `if ... { ... } else { ... }`
    - [x] `if ... { ... } else if ...`
    - [x] Type checking.
- [x] For statement
    - [x] `for <pre-st>; <condition>; <post-st> { ... }`
    - [x] `for <pre-st>; ; <post-st> { ... }`
    - [x] `for <pre-st>; ; { ... }`
    - [x] `for ; <condition>; { ... }`
    - [x] `for ; ; { ... }`
    - [x] `for <condition> { ... }`
    - [ ] `{ break }`
    - [ ] `{ continue }`
    - [x] Type checking.
- [ ] Switch statement
    - [ ] `switch <pre-st>; <expr> { ... }`
    - [ ] `case <const>:`
    - [ ] `default`
    - [ ] `break` or `fallthrough`?
- [ ] Goto statement
    - [ ] `goto <label>`
    - [ ] `label:`
- [x] Arithmetic expressions
    - [x] `-a`
    - [x] `+a`
    - [ ] `--a` (L, TBD)
    - [ ] `++a` (L, TBD)
    - [ ] `a--` (L, TBD)
    - [ ] `a++` (L, TBD)
    - [x] `a + b`
    - [x] `a - b`
    - [x] `a * b`
    - [x] `a / b`
    - [x] `a % b`
    - [x] `a = b`
    - [x] `a += b`
    - [x] `a -= b`
    - [x] `a *= b`
    - [x] `a /= b`
- [x] Bitwise expressions
    - [x] `~n`
    - [x] `a ^ b`
    - [x] `a & b`
    - [x] `a | b`
    - [x] `a << b`
    - [x] `a >> b`
    - [x] `a &= b`
    - [x] `a |= b`
    - [x] `a ^= b`
- [x] Comparison expressions
    - [x] `a < b`
    - [x] `a > b`
    - [x] `a <= b`
    - [x] `a >= b`
    - [x] `a == b`
    - [x] `a != b`
- [x] Boolean expressions
    - [x] `!n`
    - [x] `a && b`
    - [x] `a || b`
- [x] ASTNode/Type checking

# To be determined

Language details to be determined.

- [ ] No semicolons
    - REQUIREMENT: No whitespace significance (cannot be determined by new-line characters).
    - Semicolon after `return` can be determined by whether function has an output argument.
    - Semicolon after function input arguments where a return type without parens might occur is problematic.
        - `func (a int32);? int32;?`
        - Make return value to be required (introducing `void`)?
        - Make return value to be always wrapped in parens? `func (a int32) (int32)`?
- [ ] Multiple function output arguments
    - In C implemented as non-const pointer parameters.
    - Really wanted only for error reporting.
        - Zero, one or two outputs only?
        - Single output with one additional implicit `error` output?
- [ ] Prefix and postfix `++` and `--` operators as statements or expressions?
    - These are technically `x += 1`, so assignment which is a statement.
- [x] Named function output argument(s)
    - Will attempt.
- [x] Strings
    - [x] `[]char8`, `[]char16`, `[]char32` slices
    - [x] No zero-termination guarantee.
    - [x] No `string + string` bullshit.
    - [ ] How to deal with zero-terminated strings for C APIs compatibility?
- [x] Boolean types
    - For now, we're going with what Go does: We're going with 8-bit bool, in order to protect type safety and support explicit casting.
        - `int` cannot be cast to `bool`, you need to do `i != 0`
        - `bool` cannot be cast to `int`, you need to do `if b { i == 1 }`
        - `bool` is not supported by arithmetic operations.
        - Some of these might change though after proper testing.
            - The cast from bool to int is quite safe.
            - Problem is that some arithmetic operations with bool make sense in order to do branchless calculations.
- [ ] _Go_ interfaces
- [ ] Code in root?
    - Module is function?
    - Module has root code as `main` function?
    - Pros:
        - Good for scripting.
        - Good for module initialization.
    - Cons:
        - Incompatible with C's `#include` (initialization has to be called manually)
- [ ] Context as silent argument?