# To do (messy)

See README.md for high-level tasks.

# Declarations

- Value
    - Spec: Function
- Value
    - Spec: Variable
- Value
    - Spec: Type



## Done

- [x] Proper file name error.
- [x] Errors
    - [x] Full error reporting.
        - [x] Pass position.
        - [x] Show full error.
- [x] Refactor `Type`
    - [x] Implement instance of a function as a pointer to `Type_Func`.
    - [x] Implement a `Decl_Type` node.
    - [x] Implement a `Type` node.
- [x] Generate `Ref` node after every `Decl` node has been resolved.
    - Rename it to `Symbol`
    - It will reference the finally resolved node and the original decl.
    - Will hold the position of the instance.
- [x] Get rid of `Ref` node type?
    - It's a "marker" of where the decl was.
    - NO: We'll need it to refer to the declaration, but still hold information about the place of use (like position).
- [x] Explicit casting `int32(expr)`
    - Parsed as call, resolved as a `(expr-cast type expr)`

## Next

- [ ] Bitwise operators should only work with int* types (no bool).
    - `x & false (operator & not defined on bool)`
- [ ] Cannot cast bool to int
- [ ] `bool` is 8 bit
- [ ] Implement decl+assigment `var a int32 = b;`

- [ ] Warnings
- [ ] Packages
- [ ] Get rid of need for semicolon after `{...}` in function parser.
- [ ] Actual type checking done for expressions:
    - 'Is L a variable with numeric type?'
    - 'Is R a variable with numeric type?'
    - 'Do types of L and R match?'
- [ ] Type checking
    - [ ] Binary
    - [ ] Unary
        - Must be numeric
    - [ ] Type call
        [ ] Args
        [ ] Number of arguments
    - [ ] Expression call
        [ ] Args
        [ ] Number of arguments

- [ ] Testing error states.
- [ ] Implement `Type_Pointer`.



# Read

- https://blog.golang.org/gos-declaration-syntax
- https://talks.golang.org/2012/10things.slide

# Later

- [ ] Memory optimizations:
    [ ] Implement a different type of array, one that keeps it's properties (count, capacity) on the heap (should take less space in `RiNode` union).
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
