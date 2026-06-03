; ============================================================
; factorial.asm  -  Calcula N! de forma recursiva (MASM x86)
; Convencion: parametro N llega por el stack
;             resultado se devuelve en EAX
; ============================================================

.MODEL FLAT, STDCALL
.STACK 4096

.DATA
    msg     DB  "Resultado: %d", 0Ah, 0   ; cadena para printf
    N_val   DD  5                          ; valor de N (cambialo aqui)

.CODE

; ----------------------------------------------------------
; factorial PROC
;   Entrada:  [EBP+8] = N  (DWORD, entero sin signo)
;   Salida:   EAX = N!
; ----------------------------------------------------------
factorial PROC
    push    EBP              ; guardar frame anterior
    mov     EBP, ESP         ; establecer nuevo frame

    push    EBX              ; guardar EBX (lo usaremos)

    mov     EBX, [EBP+8]     ; EBX = N

    ; --- caso base: si N == 0, retornar 1 ---
    cmp     EBX, 0
    jne     caso_recursivo
    mov     EAX, 1           ; factorial(0) = 1
    jmp     fin_factorial

caso_recursivo:
    ; --- llamada recursiva: factorial(N-1) ---
    mov     EAX, EBX
    dec     EAX              ; EAX = N - 1
    push    EAX              ; pasar N-1 como argumento
    call    factorial        ; EAX = factorial(N-1)
    add     ESP, 4           ; limpiar argumento del stack

    ; --- EAX = N * factorial(N-1) ---
    imul    EAX, EBX         ; EAX = resultado_anterior * N

fin_factorial:
    pop     EBX              ; restaurar EBX
    pop     EBP              ; restaurar frame
    ret                      ; devolver EAX al llamador
factorial ENDP


; ----------------------------------------------------------
; main: punto de entrada
; ----------------------------------------------------------
main PROC
    ; llamar factorial(N_val)
    push    N_val            ; pasar N como argumento
    call    factorial
    add     ESP, 4           ; limpiar argumento

    ; EAX tiene el resultado
    ; (aqui podrias imprimir con printf o guardar en memoria)
    ; Para mostrar en consola con C runtime:
    push    EAX              ; segundo argumento: el resultado
    push    OFFSET msg       ; primer argumento: cadena formato
    call    printf
    add     ESP, 8

    ret
main ENDP

END main