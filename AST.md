# AST

## `Spec` nodes
Actual definition of one of language's entities:

- Types
    - Numeric
        - Boolean
        - Signed integer
        - Unsigned integer
        - Floating point
    - Function
    - Pointer
    - Compound
        - Struct
        - Union
- Functions
- Variables

## `Decl` nodes
Instances of spec declaration (assigning a symbol name to language entity spec).

## `Value` nodes
Symbols resolve to values.

- Specs resolved from symbols through declarations.
- Specs defined in place.
- Constants
