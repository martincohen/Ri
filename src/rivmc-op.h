INST(None, "none")

INST(Nop, "nop")
INST(Ret, "ret")

// Assign(A = B)
INST(Assign, "assign")
// Store(Memory[A + B] = C)
// Stores sizeof(B) bytes to memory address A + C.
INST(Store, "store")
// Load(C = Memory[A + B])
// Loads sizeof(B) bytes from memory address A + C.
INST(Load, "load")

// Push(A)
// Pushes A to the arguments stack.
INST(Push, "push")
// PopN(A)
// Pops A count of arguments.
INST(PopN, "pop-n")

// A = Call(Func B)
// Calls function B and sets result to A.
// If A.Type == None, result is ignored.
INST(Call, "call")
// A = CallAddr(Addr B)
INST(CallAddr, "call-addr")

// (goto A)
INST(GoTo, "goto")
// (if (A == 0) goto B else goto C)
INST(If, "if")

GROUP_START(Binary)
    INST(Binary_Add, "+")
    INST(Binary_Sub, "-")
    INST(Binary_Mul, "*")
    INST(Binary_Div, "/")
    INST(Binary_Mod, "%")
    INST(Binary_BXor, "^")
    INST(Binary_BAnd, "&")
    INST(Binary_BOr, "|")
    INST(Binary_BShL, "<<")
    INST(Binary_BShR, ">>")
    INST(Binary_And, "&&")
    INST(Binary_Or, "||")
    GROUP_START(Binary_Comparison)
        INST(Binary_Comparison_Lt, "<")
        INST(Binary_Comparison_Gt, ">")
        INST(Binary_Comparison_LtEq, "<=")
        INST(Binary_Comparison_GtEq, ">=")
        INST(Binary_Comparison_Eq, "==")
        INST(Binary_Comparison_NotEq, "!=")
    GROUP_END(Binary_Comparison)
GROUP_END(Binary)