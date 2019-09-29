# Ri

_Ri_ is statically typed embeddable language.

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
- `ri-to-c.c`
    - Program
    - Take _RiAST_, output C file(s).
    - Run compiler.
    - Run executable.

# To be determined

Language details to be determined.

- [ ] Multiple function output arguments
    - In C implemented as non-const pointer parameters.
    - Really wanted only for error reporting.
        - Zero, one or two outputs only?
        - Single output with one additional implicit `error` output?
- [x] Named function output argument(s)
- [x] Strings
    - [x] `[]char8`, `[]char16`, `[]char32` slices
    - [x] No zero-termination guarantee.
    - [x] No `string + string` bullshit.
    - [ ] How to deal with zero-terminated strings for C APIs compatibility?
- [ ] Boolean types
    - 32-bit bool, or 1-bit bool?
    - Other options?
- [ ] _Go_ interfaces

# To do

The work is split to phases:

- **Phase 1** AST: Basics
    - Lexer, parser and resolver basics
    - All statements
    - Simple scalar types
- **Phase 2** AST: Pointer, Struct, Union, Initializers, Enum
- **Phase 3** AST: Slices
    - C arrays with count
- Integration with _Runt_ using simple AST interpreter for initializers.
- Packages
- Compiling to C
- Compiling to VM
- Compiling to x64

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
    - [ ] Boolean type(s)
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
- [x] Function declaration
    - [x] `function <name> (<inputs>) (<outputs>) { ... }`
- [x] Function prototype declaration
    - [x] `function <name> (<inputs>) (<outputs>)`
- [x] Call
    - [x] `<name>(<arguments>)`
    - [ ] Type checking.
- [x] If statement
    - [x] `if <pre-st>; <condition> { ... }`
    - [x] `if <condition> { ... }`
    - [x] `if ... { ... } else { ... }`
    - [x] `if ... { ... } else if ...`
    - [ ] Type checking.
- [x] For statement
    - [x] `for <pre-st>; <condition>; <post-st> { ... }`
    - [x] `for <pre-st>; ; <post-st> { ... }`
    - [x] `for <pre-st>; ; { ... }`
    - [x] `for ; <condition>; { ... }`
    - [x] `for ; ; { ... }`
    - [x] `for <condition> { ... }`
    - [ ] Type checking.
- [ ] Arithmetic expressions
    - [ ] `-a`
    - [ ] `+a`
    - [x] `a + b`
    - [x] `a - b`
    - [ ] `a * b`
    - [ ] `a / b`
    - [ ] `a % b`
    - [x] `a = b`
    - [x] `a += b`
    - [x] `a -= b`
    - [ ] `a *= b`
    - [ ] `a /= b`
- [ ] Bitwise expressions
    - [ ] `~n`
    - [ ] `a ^ b`
    - [ ] `a & b`
    - [ ] `a | b`
    - [ ] `a << b`
    - [ ] `a >> b`
    - [ ] `a &= b`
    - [ ] `a |= b`
    - [ ] `a ^= b`
- [ ] Comparison expressions
    - [ ] `a < b`
    - [ ] `a > b`
    - [ ] `a <= b`
    - [ ] `a >= b`
    - [ ] `a == b`
    - [ ] `a != b`
- [ ] Boolean expressions
    - [ ] `!n`
    - [ ] `a && b`
    - [ ] `a || b`
- [ ] Type checking, coercion and promotion for expressions.
