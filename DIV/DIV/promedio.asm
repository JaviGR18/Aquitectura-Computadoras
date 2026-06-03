.586
.model flat, stdcall 
.stack 2048
ExitProcess PROTO, exitCode: DWORD

.data
    
    datos SDWORD -7, -2, -5, -1, -3, 0

.code
main proc
    MOV EAX, 0
    MOV EBX, 0
    MOV ECX, LENGTHOF datos

ini_loop:
    ADD EAX, datos[EBX]
    ADD EBX, 4
    LOOP ini_loop

    
    MOV ECX, LENGTHOF datos
    CDQ                 
    IDIV ECX            

    INVOKE ExitProcess, 0
main endp
END main