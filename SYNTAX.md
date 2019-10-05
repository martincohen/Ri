# `Ri` syntax

This document reflects current state of implementation. See _To do_ section in main `README.md`.

Syntax is heavily inspired by [Go](https://golang.org/ref/spec), but not the same. Differences so far:

- No support for short variable declaration `a := x`, do `var a = x` instead.
- No `int` type, use explicit `int32` or `int64`.
- `return` statement is either standalone `return` or with call arguments `return (...)`.
- No built-in `string` type, use `[]char8`, `[]char16`, `[]char32` instead.
- No `+` operator for string concatenation.
- No `&^` (bit-clear, AND NOT) operator.
- Bitwise complement `^x` is implemented as `~x`.

Differences from C so far:

- Strings are slices, not pointers.
- No string zero-termination guarantee.
- No comma operator.
- No `while` statement, use `for`.
- No `do-while` statement, use `for`.

## Types

Integer types:

- `int8`
- `uint8`
- `int16`
- `uint16`
- `int32`
- `uint32`
- `int64`
- `uint64`

Character types:

- `char8` (alias `int8`)
- `char16` (alias `int16`)
- `char32` (alias `int32`)

Float types:

- `float32`
- `float64`

## Syntax

### Semicolons

Currently required with rules similar to C. Will be re-evaluated to see whether they can be removed.

## Declarations

### `function` declaration

- `function <name> (arg, arg,...) (arg, arg, ...) {...}`

#### Function arguments

- `<name> <type>`

### `var` declaration

- `var <name> <type>`

## Statements

### Simple statement

- Expression
- Var declaration
- Assignment

### `return` statement

- `return ...;`

To be changed to:

- `return (...)`
- `return`

### `if` statement

- `if <pre-st>; <condition> { ... }`
- `if <condition> { ... }`
- `if ... { ... } else { ... }`
- `if ... { ... } else if ...`

Where:
- `pre-st` is simple statement
- `condition` is expression

### `for` statement

- `for <pre-st>; <condition>; <post-st> { ... }`
- `for <pre-st>; ; <post-st> { ... }`
- `for <pre-st>; ; { ... }`
- `for ; ; <post-st> { ... }`
- `for ; <condition>; { ... }`
- `for ; ; { ... }`
- `for <condition> { ... }`

Where:
- `pre-st` is simple statement
- `post-st` is simple statement
- `condition` is expression

#### `while` statement

```go
for condition {
    work();
}
```

#### `do-while` statement

```go
for var ok = true; ok; ok = condition {
    work();
}
```

#### `repeat-until` statement

```go
for var ok = true; ok; ok = !condition {
    work();
}
```

### Assignment statements

- `a = b`
- `a += b`
- `a -= b`
- `a *= b`
- `a /= b`
- `a %= b`
- `a &= b`
- `a |= b`
- `a ^= b`

## Expressions

### Call

- `<name>(...)`

### Arithmetic operators

- `a + b`
- `a - b`
- `a * b`
- `a / b`
- `a % b`

### Bitwise operators

- `~n`
- `a & b`
- `a | b`
- `a ^ b`
- `a << b`
- `a >> b`

### Comparison operators

- `a == b`
- `a != b`
- `a < b`
- `a <= b`
- `a > b`
- `a >= b`

### Boolean operators

- `!a`
- `a && b`
- `a || b`