# To do (messy)

## Read

- https://github.com/luciotato/golang-notes/blob/master/OOP.md
- https://github.com/orangeduck/Cello
- https://github.com/orangeduck/mpc
- https://www.youtube.com/watch?v=eF9qWbuQLuw&list=PLzLzYGEbdY5n9ITKUqOuRjXkRU5tMW2Sd&index=2
- https://blog.golang.org/gos-declaration-syntax
- https://talks.golang.org/2012/10things.slide

See README.md for high-level tasks.

## Next

### VM

GOAL: Compiles into three-address code, interprets three-address code in a straightforward manner. (baseline performance)


- [x] Draft instruction set
- [x] Draft functions
- [x] Draft values (var, const)
- [x] Draft `+` binary operator
- [x] Draft `if` compile
- [x] Support for constant types? (resolved in AST)
- [x] Draft calls
- [x] Merge `call` and `arg-pop-n`.
- [x] Draft VM execution
- [x] Baseline compiler infrastructure.
- [x] Call compilation.
- [x] Calling C functions.

- [ ] Compile "root" as module-init function.
- [ ] Draft constant folding
- [ ] Draft x64 compilation
- [ ] Allow per-function compilation so we can try compile-time execution.
- [ ] Casting.
    - signed -> unsigned
    - unsigned -> signed
    - float promotion
    - float demotion
    - float32 -> integer (trunc_signed, trunc_unsigned)
    - float64 -> integer (trunc_signed, trunc_unsigned)
    - integer -> float32
    - integer -> float64
    - integer promotion
    - integer demotion
    - reinterpret

- [ ] Either distinguish between VM and C functions when calling, or make VM functions called via standard C calling convention.
    - Each function will be wrapped in a generated x64 code that translates arguments array to registers (and stack) and calls `rivm_exec`.
- [ ] Draft register allocation
    - 2 variants:
        a) Allocate all parameters in registers, the rest goes to memory.
            - Use virtual slots to do the allocation so the strategy can be changed.
        b) Use two-register stack machine unwinding.
            - All variables are in "stack" memory.
            - We pull and push as we need.
            - We keep the two top-most in registers.

### AST

- [x] Ignore named return values (we'll do multiple return values and named return values later).
- [ ] Ri calling convention passing a pointer to current `context` as first argument.
    - This will be simply determined by the signature of the function we're calling, if the signature has `*RiContext` as first argument, we'll inject it as one, otherwise we'll just call using platform's native calling convention.
- [ ] Rework `constant` node as it's messy now. Probably also add support for initializers.
- [ ] Merge same types. See assignability rules here: https://golang.org/ref/spec#Assignability
- [ ] `nil` as predefined identifier
- [ ] Check for compatible types when casting consts.
    - See https://golang.org/ref/spec#Representability
- [ ] Type checking
    - [x] Basic type checking.
    - [x] Constant implicit casting.
    - [ ] Type checking for `for`.
    - [ ] Type checking for `if`.
    - [ ] Type checking for `switch`.
    - [ ] Type checking for `return`.
        - [x] Implicit cast for constants in argument expression.
        - [ ] Check if return's argument type is the same as function's return type.
        - [ ] Check if return has been called in functions having a return type.
    - [ ] Type checking for calls.
        - [ ] Ensure number of arguments is the same.
    - [ ] Type inference for variable declarations `var x = <expr>;`.

- [ ] Limit use of `=` for type inference, so we can't do `var a += 1`, which is now legal.
- [ ] Limit use of `Type_Infer` variables before they are actually declared and assigned.
    - Is this actual "uninitialized use of variable" protection?
- [ ] Text ranges in nodes:
    - `RiToken` to also hold Range or at least `length`
    - `RiNode` will contain pos + length caclulated from the token.
- Constants
    - [x] Integer constants
    - [x] Typed constants
    - [ ] Named typed constants
    - [ ] Real constants
    - [ ] String constants
    - [ ] Struct initializer constants
    - [ ] Union initializer constants
- [ ] Allow multiple errors in `typecheck` phase.
    - `ri_error_add_` will add an error.
    - Change `ri_error_set_` to `ri_fatal_set_`

- [ ] Parsing type spec.
    - Parsing `func (...) (...)` as function type spec.
    - Parsing `struct {...}` as struct type spec.
- [ ] Parsing type declaration.
    - `type <name> <type-spec>`
    - `type <name> = <other-name>` for declaring type that implicitly casts to `other-name`
    ` `type <name> <other-name>` for declaring new type that has same semantics, but doesn't implicitly cast to `other-name`
- [ ] Implement decl+assigment `var a int32 = b;`
- [x] Implement `switch..case` statement
    - [ ] Use `fallthrough` instead of `break`?
        - Pro: `break` will be then unique to `for`.

- [ ] `Pointer`
- [ ] `Struct`
- [ ] `Union`
- [ ] `Enum`

- [ ] Packages
    - Directory works as a package
    - All files in directory are treated as if they were a single file.
        - All declarations are visible.
    - Explicit exports?
        - Use `_` prefix to mark functions that should not export.
        - Tests can still access these (compiler only issues a warning).
        - Are test going to be part of the package? Would make sense.
- [ ] Get rid of need for semicolon after `{...}` in function parser.
- [ ] Warnings

- [ ] Testing error states.

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

# Future

- Enums
    - Stringifying enums.
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
- Do we want `??` operator?
    - https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/operators/null-coalescing-operator
- Compound math types
    - I learnt the other week that it's called level 1 BLAS (basic linear algebra system). Level 2 is matrix vector multiplication. Level 3 is matrix matrix multiplication
    - We'd need a fixed-count slice for this to work.

    - Difference between
        - static size slice with items allocated on stack
        - infered-but-static size slice with items allocated on stack
        - dynamic size

struct T {
    var items T;
    const count int64 = 2;
}

```go

var a []float;
var b (2)float;
var c ()float = { ... };

var a []float;
var b [2]float;
var c [*]float = { ... };

var a Slice(float);
var b [2]float;
b = { 1, 2 };
var c []float = { ... };


// Impossible because `[]type` is slice.
const a [*]float = {
    1, 2, 3
}
// Type-infered constant.
const a [] = {
    1, 2, 3
}


var a, b [2]float32;
var c = a * b;

type Vector2 [2]float32;
var a, b Vector2;
var c = a + b;
```

## Future / Packages
- Package is a function that returns a struct-like instance.
```go
// lib.ri
func file_read(path char8[]) uint8[] {
    // ...
}

// main.ri
import "lib"
var data = lib.file_read("test.txt");
```