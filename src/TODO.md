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
- [x] Bitwise operators should only work with int* types (no bool).
    - `x & false (operator & not defined on bool)`
- [x] Allow casting bool to int and float
- [x] `bool` is 8 bit
- [x] Actual type checking done for expressions:
- [x] Type checking for everything except function arguments.

## Next

### VM

- [x] Draft instruction set
- [x] Draft functions
- [x] Draft values (var, const)
- [x] Draft `+` binary operator
- [x] Draft `if` compile

- [ ] Fix bug in AST (see bellow), then enable `op-binary` tests.
- [ ] Compile "root" as module-init function.
- [ ] Draft constant folding
- [ ] Draft calls
    - `Push <value>`
    - `Push <value>`
    - `Push <value>`
    - `Call <func>`
    - `Drop <count>`
- [ ] Draft VM execution
- [ ] Draft x64 compilation

### AST

- [ ] Limit use of `=` for type inference, so we can't do `var a += 1`, which is now legal.
- [ ] Limit use of `Type_Infer` variables before they are actually declared and assigned.
    - Is this actual "uninitialized use of variable" protection?
- [ ] `typecheck` phase:
    - [x] Constant type inference infrastructure
    - [ ] Implement `var x = <expr>;` for type inference.
    - [ ] In `resolve` phase make sure declarations in assignments are processed.
    - [ ] Allow multiple errors in `typecheck` phase.
        - `ri_error_add_` will add an error.
        - Change `ri_error_set_` to `ri_fatal_set_`
    - [ ] Move type checks to `typecheck` phase.
        - `ri_resolve_expr_call_type_`
    - [ ] Type checking for calls.
- [ ] Text ranges in nodes:
    - `RiToken` to also hold Range or at least `length`
    - `RiNode` will contain pos + length caclulated from the token.
- [x] Integer constants
- [x] Typed constants
- [ ] Named typed constants
- [ ] Real constants
- [ ] String constants
- [ ] Initializer constants

- [ ] Parsing type spec.
    - Parsing `func (...) (...)` as function type spec.
    - Parsing `struct {...}` as struct type spec.
- [ ] Parsing type declaration.
    - `type <name> <type-spec>`
    - `type <name> = <other-name>` for declaring type that implicitly casts to `other-name`
    ` `type <name> <other-name>` for declaring new type that has same semantics, but doesn't implicitly cast to `other-name`
- [ ] Implement decl+assigment `var a int32 = b;`
- [ ] Implement `switch..case` statement
    - Use `break` or `fallthrough`?

- [ ] Packages
    - Directory works as a package
    - All files in directory are treated as if they were a single file.
        - All declarations are visible.
    - Explicit exports?
        - Use `_` prefix to mark functions that should not export.
        - Tests can still access these (compiler only issues a warning).
        - Are test going to be part of the package? Would make sense.
- [ ] Get rid of need for semicolon after `{...}` in function parser.
- [ ] Type checking for call arguments.
- [ ] Warnings

- [ ] Testing error states.
- [ ] Implement `Type_Pointer`.

# Read

- https://www.youtube.com/watch?v=eF9qWbuQLuw&list=PLzLzYGEbdY5n9ITKUqOuRjXkRU5tMW2Sd&index=2
- https://blog.golang.org/gos-declaration-syntax
- https://talks.golang.org/2012/10things.slide

# Later

- [ ] Non-critical errors should not panic.
    - Non-critical means we can fix the errors temporarily so they won't cause more cascading problems.
    - For example skip entire statements when parsing, or fix casting errors.
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
- [ ] Collapse FIRST and LAST values in enums.
- [ ] Implement this?
        - `var main function(...)`
        - `main = { ... }`

## Interpreter

- Compile first to WASM.
- Compile to straightforward x64
- Register allocation
    - 2 variants:
        a) Allocate all parameters in registers, the rest goes to memory.
            - Use virtual slots to do the allocation so the strategy can be changed.
        b) Use two-register stack machine unwinding.
            - All variables are in "stack" memory.
            - We pull and push as we need.
            - We keep the two top-most in registers.

# Future

- Enums
- Interfaces
    - There's a dynamic property to interfaces which requires type knowledge at runtime.
    - https://medium.com/rungo/interfaces-in-go-ab1601159b3a
    - https://research.swtch.com/interfaces
    - https://golang.org/doc/effective_go.html#interfaces
- Multiple return values
    - On C side it's returned as `struct`.
- Slice
    - `[]int`
    - Syntax sugar for `struct { T* items; iptr count; }`
