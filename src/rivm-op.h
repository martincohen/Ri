RIVM_INST(None, "none")

RIVM_INST(Nop, "nop")
RIVM_INST(Ret, "ret")

RIVM_INST(Enter, "enter")
RIVM_INST(Leave, "leave")

// Assign(A = B)
RIVM_INST(Assign, "assign")
// Store(Memory[A + B] = C)
// Stores sizeof(B) bytes to memory address A + C.
RIVM_INST(Store, "store")
// Load(C = Memory[A + B])
// Loads sizeof(B) bytes from memory address A + C.
RIVM_INST(Load, "load")
// A = AddrOf(B)
RIVM_INST(AddrOf, "addr-of")

// Arg(Value)
// Pushes Value to the stack.
RIVM_INST(ArgPush, "arg-push")
// PopN(Count)
// Pops A count of arguments.
RIVM_INST(ArgPopN, "arg-pop-n")

// A = Call(Func B)
// Calls function B and sets result to A.
// If A.Type == None, result is ignored.
RIVM_INST(Call, "call")

// (goto A)
RIVM_INST(GoTo, "goto")
// (if (A == 0) goto B else goto C)
RIVM_INST(If, "if")

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