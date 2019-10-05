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

- :white_square_button: Built-in types
    - :white_square_button: `int8`
    - :white_square_button: `uint8`
    - :white_square_button: `int16`
    - :white_square_button: `uint16`
    - :white_square_button: `int32`
    - :white_square_button: `uint32`
    - :white_square_button: `int64`
    - :white_square_button: `uint64`
    - :white_square_button: `float32`
    - :white_square_button: `float64`
    - :black_square_button: Boolean type(s)
    - :black_square_button: Complex type(s)
        - `complex64`
        - `complex128`
    - :black_square_button: SSE type(s)
    - :black_square_button: Pointers
        - :black_square_button: Function pointers
    - :black_square_button: Struct
    - :black_square_button: Union
    - :black_square_button: Enum
    - :black_square_button: Slices
        - `[]type`
- :black_square_button: Comments
    - :white_square_button: Line comments
    - :black_square_button: Block comments
- :white_square_button: Function declaration
    - :white_square_button: `function <name> (<inputs>) (<outputs>) { ... }`
- :white_square_button: Function prototype declaration
    - :white_square_button: `function <name> (<inputs>) (<outputs>)`
- :white_square_button: Call
    - :white_square_button: `<name>(<arguments>)`
    - :black_square_button: Type checking.
- :white_square_button: If statement
    - :white_square_button: `if <pre-st>; <condition> { ... }`
    - :white_square_button: `if <condition> { ... }`
    - :white_square_button: `if ... { ... } else { ... }`
    - :white_square_button: `if ... { ... } else if ...`
    - :black_square_button: Type checking.
- :white_square_button: For statement
    - :white_square_button: `for <pre-st>; <condition>; <post-st> { ... }`
    - :white_square_button: `for <pre-st>; ; <post-st> { ... }`
    - :white_square_button: `for <pre-st>; ; { ... }`
    - :white_square_button: `for ; <condition>; { ... }`
    - :white_square_button: `for ; ; { ... }`
    - :white_square_button: `for <condition> { ... }`
    - :black_square_button: Type checking.
- :black_square_button: Arithmetic expressions
    - :white_square_button: `-a`
    - :white_square_button: `+a`
    - :black_square_button: `--a` (L)
    - :black_square_button: `++a` (L)
    - :black_square_button: `a--` (L)
    - :black_square_button: `a++` (L)
    - :white_square_button: `a + b`
    - :white_square_button: `a - b`
    - :white_square_button: `a * b`
    - :white_square_button: `a / b`
    - :white_square_button: `a % b`
    - :white_square_button: `a = b`
    - :white_square_button: `a += b`
    - :white_square_button: `a -= b`
    - :white_square_button: `a *= b`
    - :white_square_button: `a /= b`
- :black_square_button: Bitwise expressions
    - :white_square_button: `~n`
    - :white_square_button: `a ^ b`
    - :white_square_button: `a & b`
    - :white_square_button: `a | b`
    - :white_square_button: `a << b`
    - :white_square_button: `a >> b`
    - :white_square_button: `a &= b`
    - :white_square_button: `a |= b`
    - :white_square_button: `a ^= b`
- :black_square_button: Comparison expressions
    - :white_square_button: `a < b`
    - :white_square_button: `a > b`
    - :white_square_button: `a <= b`
    - :white_square_button: `a >= b`
    - :white_square_button: `a == b`
    - :white_square_button: `a != b`
- :black_square_button: Boolean expressions
    - :white_square_button: `!n`
    - :white_square_button: `a && b`
    - :white_square_button: `a || b`
- :black_square_button: ASTNode/Type checksing

# To be determined

Language details to be determined.

- :black_square_button: Multiple function output arguments
    - In C implemented as non-const pointer parameters.
    - Really wanted only for error reporting.
        - Zero, one or two outputs only?
        - Single output with one additional implicit `error` output?
- :white_square_button: Named function output argument(s)
    - Will attempt.
- :white_square_button: Strings
    - :white_square_button: `[]char8`, `[]char16`, `[]char32` slices
    - :white_square_button: No zero-termination guarantee.
    - :white_square_button: No `string + string` bullshit.
    - :black_square_button: How to deal with zero-terminated strings for C APIs compatibility?
- :black_square_button: Boolean types
    - 32-bit bool, or 1-bit bool?
    - Other options?
- :black_square_button: _Go_ interfaces
- :black_square_button: Code in root?
    - Module is function?
    - Module has root code as `main` function?
    - Pros:
        - Good for scripting.
        - Good for module initialization.
    - Cons:
        - Incompatible with C's `#include` (initialization has to be called manually)
- :black_square_button: Context as silent argument?