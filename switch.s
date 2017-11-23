	AREA switch, CODE, readonly
	EXPORT firstProces
	EXPORT ctxswitch

firstProces	PROC
			mov sp, r0
            pop { r2-r6 }
            mov r12, r6 
            mov r11, r5 
            mov r10, r4 
            mov r9, r3 
			mov r8, r2
			pop { r4-r7, pc }
			ENDP
					
ctxswitch	PROC
			;sichern von p1
			push {r4-r12, lr}
			;holen von p2
			mov sp,r1 
			pop {r4-r12,pc} 
			bx lr
			ENDP