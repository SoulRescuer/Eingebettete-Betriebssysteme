	AREA switch, CODE, readonly
	EXPORT firstProcess
	EXPORT ctxswitch

firstProcess	PROC
				mov sp,r0
				pop {r4-r12,pc}		;Reihenfolge
				ENDP
					
ctxsw			PROC
				;sichern von p1
				push {r4-r12,lr}  ;Reihenfolge?
				mov r4, sp
				str r4,[r1]
				
				;holen von p2
				mov sp,r0
				pop {r4-r12,pc} ;Reihenfolge?
				ENDP
	
	

