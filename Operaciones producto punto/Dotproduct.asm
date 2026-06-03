; ============================================================
;  dotproduct.asm  -  Producto punto FPU / SSE / AVX + RDTSC
;  Arquitectura : x86-64  (ml64.exe, Visual Studio x64)
;  Convencion   : Microsoft x64 ABI
; ============================================================

_TEXT SEGMENT

; ============================================================
;  rdtsc_start  ->  RAX = TSC serializado ANTES
; ============================================================
rdtsc_start PROC PUBLIC
    PUSH    RBX
    XOR     EAX, EAX
    CPUID
    RDTSC
    SHL     RDX, 32
    OR      RAX, RDX
    POP     RBX
    RET
rdtsc_start ENDP

; ============================================================
;  rdtsc_end  ->  RAX = TSC serializado DESPUES
; ============================================================
rdtsc_end PROC PUBLIC
    RDTSC
    SHL     RDX, 32
    OR      RAX, RDX
    PUSH    RAX
    PUSH    RBX
    XOR     EAX, EAX
    CPUID
    POP     RBX
    POP     RAX
    RET
rdtsc_end ENDP

; ============================================================
;  dot_fpu(float* A, float* B, int N)  ->  float en XMM0
;  Unidad x87 FPU clasica, un float por ciclo
; ============================================================
dot_fpu PROC PUBLIC
    PUSH    RSI
    PUSH    RDI
    PUSH    RBX

    MOV     RSI, RCX            ; RSI = A
    MOV     RDI, RDX            ; RDI = B
    MOV     EBX, R8D            ; EBX = N

    FLDZ                        ; ST0 = 0.0  (acumulador)

    TEST    EBX, EBX
    JZ      fpu_done

fpu_loop:
    FLD     DWORD PTR [RSI]     ; ST0 = A[i]
    FLD     DWORD PTR [RDI]     ; ST0 = B[i]
    FMULP   ST(1), ST(0)        ; ST0 = A[i] * B[i]
    FADDP   ST(1), ST(0)        ; ST0 = acc + A[i]*B[i]
    ADD     RSI, 4
    ADD     RDI, 4
    DEC     EBX
    JNZ     fpu_loop

fpu_done:
    SUB     RSP, 8
    FSTP    DWORD PTR [RSP]
    MOVSS   XMM0, DWORD PTR [RSP]
    ADD     RSP, 8

    POP     RBX
    POP     RDI
    POP     RSI
    RET
dot_fpu ENDP

; ============================================================
;  dot_sse(float* A, float* B, int N)  ->  float en XMM0
;  Registros XMM 128 bits = 4 floats por iteracion
; ============================================================
dot_sse PROC PUBLIC
    PUSH    RSI
    PUSH    RDI
    PUSH    RBX

    MOV     RSI, RCX
    MOV     RDI, RDX
    MOV     EBX, R8D

    XORPS   XMM4, XMM4          ; acumulador = 0.0

    MOV     EAX, EBX
    SHR     EAX, 2              ; EAX = N / 4
    TEST    EAX, EAX
    JZ      sse_remainder

sse_loop:
    MOVUPS  XMM0, XMMWORD PTR [RSI]
    MOVUPS  XMM1, XMMWORD PTR [RDI]
    MULPS   XMM0, XMM1
    ADDPS   XMM4, XMM0
    ADD     RSI, 16
    ADD     RDI, 16
    DEC     EAX
    JNZ     sse_loop

sse_remainder:
    MOV     EAX, EBX
    AND     EAX, 3
    TEST    EAX, EAX
    JZ      sse_hsum

sse_rem_loop:
    MOVSS   XMM0, DWORD PTR [RSI]
    MOVSS   XMM1, DWORD PTR [RDI]
    MULSS   XMM0, XMM1
    ADDSS   XMM4, XMM0
    ADD     RSI, 4
    ADD     RDI, 4
    DEC     EAX
    JNZ     sse_rem_loop

sse_hsum:
    MOVAPS  XMM0, XMM4
    MOVHLPS XMM1, XMM0
    ADDPS   XMM0, XMM1
    SHUFPS  XMM1, XMM0, 01h
    ADDSS   XMM0, XMM1

    POP     RBX
    POP     RDI
    POP     RSI
    RET
dot_sse ENDP

; ============================================================
;  dot_avx(float* A, float* B, int N)  ->  float en XMM0
;  Registros YMM 256 bits = 8 floats por iteracion
; ============================================================
dot_avx PROC PUBLIC
    PUSH    RSI
    PUSH    RDI
    PUSH    RBX

    MOV     RSI, RCX
    MOV     RDI, RDX
    MOV     EBX, R8D

    VXORPS  YMM4, YMM4, YMM4   ; acumulador = 0.0

    MOV     EAX, EBX
    SHR     EAX, 3              ; EAX = N / 8
    TEST    EAX, EAX
    JZ      avx_remainder

avx_loop:
    VMOVUPS YMM0, YMMWORD PTR [RSI]
    VMOVUPS YMM1, YMMWORD PTR [RDI]
    VMULPS  YMM0, YMM0, YMM1
    VADDPS  YMM4, YMM4, YMM0
    ADD     RSI, 32
    ADD     RDI, 32
    DEC     EAX
    JNZ     avx_loop

avx_remainder:
    MOV     EAX, EBX
    AND     EAX, 7
    TEST    EAX, EAX
    JZ      avx_hsum

avx_rem_loop:
    VMOVSS  XMM0, DWORD PTR [RSI]
    VMOVSS  XMM1, DWORD PTR [RDI]
    VMULSS  XMM0, XMM0, XMM1
    VADDSS  XMM4, XMM4, XMM0
    ADD     RSI, 4
    ADD     RDI, 4
    DEC     EAX
    JNZ     avx_rem_loop

avx_hsum:
    VEXTRACTF128 XMM0, YMM4, 1
    VADDPS       XMM0, XMM0, XMM4
    VMOVHLPS     XMM1, XMM1, XMM0
    VADDPS       XMM0, XMM0, XMM1
    VSHUFPS      XMM1, XMM0, XMM0, 01h
    VADDSS       XMM0, XMM0, XMM1
    VZEROUPPER

    POP     RBX
    POP     RDI
    POP     RSI
    RET
dot_avx ENDP

_TEXT ENDS
END