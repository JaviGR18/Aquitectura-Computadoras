; ============================================================
;  norma.asm
;  Calcula la norma euclidiana de un vector FLOAT de N elementos
;  usando la FPU x86 (instrucciones x87).
;
;  Formula: norma = sqrt(A[0]^2 + A[1]^2 + ... + A[N-1]^2)
;
;  Prototipo C++:
;      extern "C" float Norma(float* A, int N);
;
;  Retorna el resultado en ST(0)
; ============================================================

.686
.MODEL FLAT, C
.CODE

Norma PROC C pA:PTR REAL4, N:DWORD

    FNINIT                      ; reinicia FPU

    MOV     ESI, pA             ; ESI apunta a A[0]
    MOV     ECX, N              ; ECX = contador

    FLDZ                        ; ST(0) = 0.0

    TEST    ECX, ECX
    JLE     @@done

@@loop:
    FLD     DWORD PTR [ESI]     ; ST(0) = A[i]
    FMUL    DWORD PTR [ESI]     ; ST(0) = A[i]^2
    FADDP   ST(1), ST(0)        ; ST(0) = acum + A[i]^2

    ADD     ESI, 4
    LOOP    @@loop

@@done:
    FSQRT                       ; ST(0) = sqrt(acum)

    RET

Norma ENDP
END