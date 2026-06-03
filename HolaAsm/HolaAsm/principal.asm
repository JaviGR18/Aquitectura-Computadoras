.586
.model flat, stdcall
.stack 2048

ExitProcess PROTO exitCode: DWORD

.data
    cad BYTE 'Hola Mundo!', 0
    tam DWORD 11

.code
main PROC
    LEA EBX, cad

ini_while:
    CMP BYTE PTR [EBX], 0
    JE fin_while

    CMP BYTE PTR [EBX], 'a'
    JB fin_if

    CMP BYTE PTR [EBX], 'z'
    JA fin_if

    SUB BYTE PTR [EBX], 32

fin_if:
    INC EBX
    JMP ini_while

fin_while:
    INVOKE ExitProcess, 0

main ENDP
END main