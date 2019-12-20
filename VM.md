# Virtual machine

Interpreter uses IR produced by the compiler to execute the code. This serves as a reference implementation and testing the whole ensemble.

- **Phase 1** Reference implementation
- **Phase 2** Optimizations over IR
- **Phase 2** x64 code generation

## Performace log

### Reference
Measured on AMD Threadripper 3970x.

- `c/c4` 3067.38ms
- `js/duktape` 11668.6ms
- `lua/lua51` 1422.3ms
- `lua/lua53` 1629.54ms
- `lua/lua54` 0.977ms
- `gml/gml` 19513.8ms
- `wren/wren` 2643.12ms

### 2019/19/12
- Interprets from unoptimized three-address code IR.
- `fib34.ri 5702887 (927.354ms, MSVC2019, release build)`
- `fib34.ri 5702887 (2177.013ms, MSVC2019, debug build)`
