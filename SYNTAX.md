# `Ri` syntax

## Semicolons

Currently required with rules similar to C. Will be re-evaluated to see whether they can be removed.

## Declarations

### `function` declaration

- `function <name> (arg, arg,...) (arg, arg, ...) {...}`

#### Function arguments

- `<name> <type>`

### `var` declaration

- `var <name> <type>`

## Statements

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

### `for` statement

- `for <pre-st>; <condition>; <post-st> { ... }`
- `for <pre-st>; ; <post-st> { ... }`
- `for <pre-st>; ; { ... }`
- `for ; <condition>; { ... }`
- `for ; ; { ... }`
- `for <condition> { ... }`

#### `do-while` statement

```go
for var ok = true; ok; ok = condition {
    work()
}
```

#### `repeat-until` statement

```go
for var ok = true; ok; ok = !condition {
    work()
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