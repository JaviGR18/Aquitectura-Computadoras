; ============================================================
;  dotproduct.asm
;  Producto punto de dos vectores FLOAT usando FPU x87
;  Prototipo: extern "C" float DotProduct(float* A, float* B, int N);
; ============================================================

.686
.MODEL FLAT, C
.CODE

DotProduct PROC C, pA:PTR REAL4, pB:PTR REAL4, N:DWORD

    PUSH    EBP
    MOV     EBP, ESP
    PUSH    ESI
    PUSH    EDI

    MOV     ESI, pA             ; ESI apunta a A[0]
    MOV     EDI, pB             ; EDI apunta a B[0]
    MOV     ECX, N              ; ECX = contador

    FLDZ                        ; ST(0) = 0.0  (acumulador)

    TEST    ECX, ECX
    JLE     @@done

@@loop:
    FLD     DWORD PTR [ESI]     ; ST(0)=A[i],       ST(1)=acum
    FMUL    DWORD PTR [EDI]     ; ST(0)=A[i]*B[i],  ST(1)=acum
    FADDP   ST(1), ST(0)        ; ST(0)=acum + A[i]*B[i]  (pop)

    ADD     ESI, 4              ; siguiente float de A
    ADD     EDI, 4              ; siguiente float de B
    LOOP    @@loop

@@done:
    ; resultado queda en ST(0)

    POP     EDI
    POP     ESI
    POP     EBP
    RET

DotProduct ENDP
END