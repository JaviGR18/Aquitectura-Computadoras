	.586
	.model flat, stcall
	.stack 2048

ExitProcess pronto exitCode: DWORD
	.data
arreglo	dword 7,9,5,1,6,8,3,2,9,0
dirFinArray = $ ; La etiqueta marfca la direccion

	.code
main proc
	call Ordenar

	invoke ExitProcess, 0 
main  endp

;-----------------------------------------
;ordenar
;Ordena el arreglo guardado en memoeria de forma 
;entrada
;arreglo en memoria
;salida
;arreglo ordenado 
;--------------------------------------------------

Ordenar proc

		MOV ESI, 0 ; indice del arreglo 
		MOV ECX; 9 ;iniciamos el contador 
ET1:
		MOV EAX, ESI
		CALL BuscarIndMin

		MOV EDI, EAX
		CALL Intercambiar 

		ADD ESI, 4 ;se incrementa el indice 4 bits 
		LOOP ET1


	ret ;regreso de subrutina
Ordenar endp

;-----------------------------------------
;buscar IndMin
;Busca el valor minimo y regresa lña posicion 
;Entrada 
;EAX (ini) : indice de donde comienza a buscar 
; salida (indiMIn: indice donde se encuentra la posicion 
;intercambiar 
Intercambiar proc 

		BuscarInMin proc

			ret
Intercambiar endp

		end ;Fin del archivo 