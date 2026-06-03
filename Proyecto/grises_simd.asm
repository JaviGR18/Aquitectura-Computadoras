; ============================================================
; grises_simd.asm
; Proyecto 1 - Arquitectura de Computadoras
; Convierte imagen RGB a escala de grises usando SIMD (SSE2)
; Formula: Y = 0.299*R + 0.587*G + 0.114*B
; ============================================================

.686
.XMM
.MODEL FLAT, C

.DATA
    PESO_R  REAL4   0.299       ; Peso del canal Rojo
    PESO_G  REAL4   0.587       ; Peso del canal Verde
    PESO_B  REAL4   0.114       ; Peso del canal Azul

.CODE

; ------------------------------------------------------------
; VOID convertir_a_grises(BYTE* src, BYTE* dst, INT total)
; [EBP+8]  = src  -> puntero a imagen RGB de entrada
; [EBP+12] = dst  -> puntero a imagen grises de salida
; [EBP+16] = total_pixeles
; ------------------------------------------------------------
convertir_a_grises PROC

    ; --- PROLOGO: guardar registros y establecer marco de pila ---
    PUSH    EBP
    MOV     EBP, ESP
    PUSH    ESI
    PUSH    EDI
    PUSH    EBX

    ; --- CARGAR PARAMETROS DE LA PILA ---
    MOV     ESI, [EBP+8]        ; ESI = puntero src (imagen RGB)
    MOV     EDI, [EBP+12]       ; EDI = puntero dst (imagen grises)
    MOV     ECX, [EBP+16]       ; ECX = total de pixeles (contador)

    ; --- CARGAR PESOS EN REGISTROS XMM ---
    MOVSS   XMM5, PESO_R        ; XMM5 = 0.299 (peso Rojo)
    MOVSS   XMM6, PESO_G        ; XMM6 = 0.587 (peso Verde)
    MOVSS   XMM7, PESO_B        ; XMM7 = 0.114 (peso Azul)

LOOP_PIXEL:
    ; --- VERIFICAR SI QUEDAN PIXELES ---
    TEST    ECX, ECX
    JZ      FIN_LOOP            ; Si ECX = 0, terminar

    ; --- LIMPIAR REGISTRO DE TRABAJO ---
    XOR     EAX, EAX

    ; --- LEER Y PROCESAR CANAL R ---
    MOV     AL, [ESI]           ; AL = componente R del pixel actual
    CVTSI2SS XMM0, EAX          ; XMM0 = (FLOAT) R
    MULSS   XMM0, XMM5          ; XMM0 = R * 0.299

    ; --- LEER Y PROCESAR CANAL G ---
    MOV     AL, [ESI+1]         ; AL = componente G
    CVTSI2SS XMM1, EAX          ; XMM1 = (FLOAT) G
    MULSS   XMM1, XMM6          ; XMM1 = G * 0.587

    ; --- LEER Y PROCESAR CANAL B ---
    MOV     AL, [ESI+2]         ; AL = componente B
    CVTSI2SS XMM2, EAX          ; XMM2 = (FLOAT) B
    MULSS   XMM2, XMM7          ; XMM2 = B * 0.114

    ; --- SUMAR LOS TRES TERMINOS: Y = R*0.299 + G*0.587 + B*0.114 ---
    ADDSS   XMM0, XMM1          ; XMM0 = R*0.299 + G*0.587
    ADDSS   XMM0, XMM2          ; XMM0 = Y (resultado final en flotante)

    ; --- CONVERTIR Y DE FLOTANTE A ENTERO ---
    CVTSS2SI EAX, XMM0          ; EAX = (INT) Y

    ; --- SATURAR EN RANGO [0, 255] ---
    CMP     EAX, 255
    JLE     NO_SATURAR
    MOV     EAX, 255
NO_SATURAR:
    CMP     EAX, 0
    JGE     NO_NEGATIVO
    XOR     EAX, EAX
NO_NEGATIVO:

    ; --- ESCRIBIR RESULTADO EN IMAGEN DE SALIDA ---
    MOV     [EDI], AL           ; Guardar byte de gris en DST

    ; --- AVANZAR PUNTEROS AL SIGUIENTE PIXEL ---
    ADD     ESI, 3              ; SRC avanza 3 bytes (R+G+B)
    ADD     EDI, 1              ; DST avanza 1 byte (Y)

    ; --- DECREMENTAR CONTADOR Y REPETIR ---
    DEC     ECX
    JMP     LOOP_PIXEL

FIN_LOOP:
    ; --- EPILOGO: restaurar registros y retornar ---
    POP     EBX
    POP     EDI
    POP     ESI
    POP     EBP
    RET

convertir_a_grises ENDP

END