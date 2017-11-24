	AREA switch, CODE, readonly
	EXPORT firstProces
	EXPORT ctxswitch

firstProces	PROC
			mov sp, r0
            pop { r4-r11 }
			pop { pc }
			ENDP
					
ctxswitch	PROC
			;sichern von p1
			push {r4-r11, lr}
			str	sp, [r0]
			;holen von p2
;;;; HIER!
			mov sp,r1 
			pop {r4-r11}
			pop {lr} 
			bx lr
			ENDP