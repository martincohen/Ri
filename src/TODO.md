# To do (messy)

See and update README.md for public tasks.

- [ ] Get rid of `Ref` node type?
    - It's a "marker" of where the decl was.
- [ ] Errors
    - [ ] Full error reporting.
        - [ ] Pass position.
        - [ ] Show full error.
    - [ ] Testing error states.
- [ ] Implement `Type_Pointer`.
- [ ] Refactor `Type`
    - [ ] Implement instance of a function as a pointer to `Type_Func`.
    - [ ] Implement a `Decl_Type` node.
    - [ ] Implement a `Type` node.

# Later

- [ ] Use `longjmp` for error handling. Register arrays to `ri->array`:
```c
    Array(RiNode*) arguments = {0};
    ri_array_push_(ri, &arguments);
    // ... read arguments to the array
    ri_array_pop_(ri);
    return ri_make_call_(ri, arguments); // <- registered to AST
```
    - https://en.cppreference.com/w/cpp/utility/program/setjmp
- [ ] Properly `purge` AST (dealloc arrays).
- [ ] Type inference:
    - [ ] Implement `var x = <expr>;` for type inference.
- [ ] Collapse FIRST and LAST values in enums.
- [ ] Implement this?
        - `var main function(...)`
        - `main = { ... }`

# Priorities

- x64 only
- binary C compatibility

# Features

- Numeric type promotion and coercion
- Type inference
- Flow control
    - `for statement; condition; statement { ... }`
        - `for condition { ... }` (a.k.a. while)
        - `for { ... }` (infinite for)
        - `do..while` -> https://yourbasic.org/golang/do-while-loop/
- Pointer
    - `*int`
    - Arithmetic
- Struct
    - Anonymous struct member
    - Initializer
    - Dot `struct.field`
- Union
- Functions
    - Function reference casting (pointers are the same)
- Packages

## Later

- Enums
- Interfaces
    - There's a dynamic property to interfaces which requires type knowledge at runtime.
    - https://medium.com/rungo/interfaces-in-go-ab1601159b3a
- Multiple return values
    - On C side it's returned as `struct`.
- Slice
    - `[]int`
    - Syntax sugar for `struct { T* items; iptr count; }`
