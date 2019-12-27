RIVM_INST(None, "none")
RIVM_INST(Nop, "nop")

RIVM_INST(Enter, "enter")
RIVM_INST(Ret, "ret")

// Assign(A = B)
RIVM_INST(Assign, "assign")
// Cast(A = B)
// Temporary implementation, will be replaced by a set of conversion functions:
// https://github.com/sunfishcode/wasm-reference-manual/blob/master/WebAssembly.md#conversion-instructions
RIVM_INST(Cast, "cast")

//
// Memory
//

// Store(Memory[A + B] = C)
// Stores sizeof(B) bytes to memory address A + C.
RIVM_INST(Store, "store")
// Load(C = Memory[A + B])
// Loads sizeof(B) bytes from memory address A + C.
RIVM_INST(Load, "load")
// A = AddrOf(B)
RIVM_INST(AddrOf, "addr-of")

//
// Calls
//

// Arg(Value)
// Pushes Value to the stack.
RIVM_INST(CallArg, "call-arg")
// A = Call(Func B, Imm.U64 C)
// A = Call(Imm.U64 B, Imm.U64 C)
// Calls function B, sets result to A, pops C arguments pushed with ArgPush.
// If B is of type `Func` then we're calling into a VM.
// If B is of type `U64` then we're calling into C to function at given pointer.
// If A.Type == None, result is ignored.
RIVM_INST(Call,  "call")

//
// Control
//

// (goto A)
RIVM_INST(GoTo, "goto")
// (if (A == 0) goto B else goto C)
RIVM_INST(If, "if")

//
// Arithmetic
//

// TODO: Param kind suffix: (2bit)
// I = 0
// S = 1
// ---
// - `II` b00
// - `IS` b01
// - `SI` b10
// - `SS` b11

// TODO: Type suffix (3bits)
// U  = 000
// I  = 010
// F  = 100
// 32 = 000
// 64 = 001
// ---
// - `U32` b000
// - `I32` b010
// - `U64` b001
// - `I64` b011
// - `F32` b100
// - `F64` b101

RIVM_GROUP_START(Binary)
    RIVM_INST(Binary_Add, "+")
    RIVM_INST(Binary_Sub, "-")
    RIVM_INST(Binary_Mul, "*")
    RIVM_INST(Binary_Div, "/")
    RIVM_INST(Binary_Mod, "%")
    RIVM_INST(Binary_BXor, "^")
    RIVM_INST(Binary_BAnd, "&")
    RIVM_INST(Binary_BOr, "|")
    RIVM_INST(Binary_BShL, "<<")
    RIVM_INST(Binary_BShR, ">>")
    RIVM_INST(Binary_And, "&&")
    RIVM_INST(Binary_Or, "||")
    RIVM_GROUP_START(Binary_Comparison)
        RIVM_INST(Binary_Comparison_Lt, "<")
        RIVM_INST(Binary_Comparison_Gt, ">")
        RIVM_INST(Binary_Comparison_LtEq, "<=")
        RIVM_INST(Binary_Comparison_GtEq, ">=")
        RIVM_INST(Binary_Comparison_Eq, "==")
        RIVM_INST(Binary_Comparison_NotEq, "!=")
    RIVM_GROUP_END(Binary_Comparison)
RIVM_GROUP_END(Binary)
