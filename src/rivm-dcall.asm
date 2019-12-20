; RCX size of stack-arguments
; RDX pointer to stack-arguments (packed)
; R8 pointer to register-arguments (always 4x8 bytes size)
; R9 function pointer

.CODE
rivm_dcall PROC
    OPTION PROLOGUE:NONE, EPILOGUE:NONE

    ; save registers
    push RBP
    push RSI
    push RDI

    ; save stack pointer
    mov RBP, RSP

    ; TODO: would jump over stack-copying be more efficient?

    ; align size up to 16-bytes
    ; (RCX + 15) & -16
    add RCX, 15
    and RCX, -16

    ; allocate space for stack-arguments on stack
    sub RSP, RCX

    ; copy RCX bytes from stack-arguments to stack
    ; push size bytes
    mov RSI, RDX
    mov RDI, RSP
    rep movsb

    ; remember pointer to the function
    mov RAX, R9

    ; copy register-arguments to integer registers
    mov RCX, qword ptr [R8+0]  ; argument 0
    mov RDX, qword ptr [R8+8]  ; argument 1
    ; note: change in order, because we need r8
    mov R9,  qword ptr [R8+24] ; argument 3
    mov R8,  qword ptr [R8+16] ; argument 2
    ; copy register-arguments to float registers
    movd XMM0, RCX ; argument 0 (in case it's float/double)
    movd XMM1, RDX ; argument 1 (in case it's float/double)
    movd XMM2, R8  ; argument 2 (in case it's float/double)
    movd XMM3, R9  ; argument 3 (in case it's float/double)
    ; copy register-arguments to stack
    ; todo: there doesn't need to be actual copy of the values, just 4*8 space.
    ; todo: ^ add RSP, 32
    push R9
    push R8
    push RDX
    push RCX

    ; call function pointer
    call RAX

    ; restore stack pointer
    mov RSP, RBP

    ; restore registers
    pop RDI
    pop RSI
    pop RBP

    ret
rivm_dcall ENDP
END