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

- Embeddable.
- STB-style libaries.
- Minimal binary size.
- No cstdlib dependency.
- No LLVM dependency.

- Type inference.
- No built-in memory management.
- Packages.
- Most of what C does.
- Bring your own library code.

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
- [ ] Comments
    - [x] Line comments
    - [ ] Block comments
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
    - [x] `-a`
    - [x] `+a`
    - [ ] `--a` (L)
    - [ ] `++a` (L)
    - [ ] `a--` (L)
    - [ ] `a++` (L)
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
- [ ] Bitwise expressions
    - [x] `~n`
    - [x] `a ^ b`
    - [x] `a & b`
    - [x] `a | b`
    - [x] `a << b`
    - [x] `a >> b`
    - [x] `a &= b`
    - [x] `a |= b`
    - [x] `a ^= b`
- [ ] Comparison expressions
    - [x] `a < b`
    - [x] `a > b`
    - [x] `a <= b`
    - [x] `a >= b`
    - [x] `a == b`
    - [x] `a != b`
- [ ] Boolean expressions
    - [x] `!n`
    - [x] `a && b`
    - [x] `a || b`
- [ ] Type checking, coercion and promotion for expressions.

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
- [ ] Code in root?
    - Module is function?
    - Module has root code as `main` function?
    - Pros:
        - Good for scripting.
        - Good for module initialization.
    - Cons:
        - Incompatible with C's `#include` (initialization has to be called manually)
- [ ] Context as silent argument?