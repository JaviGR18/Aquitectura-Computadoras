	.data

	.code

GetGetGenuineIntel PROC 
					
				R10 R10 RCX
				MOV RAX, 0
				CPUID
				MOV DWORD PTR [R10], EBX
				MOV DWORD PTR [R10+4], ECX
				MOV DWORD PTR [R10+8], EDX


					RET	

GetGetGenuineIntel ENDP

END
