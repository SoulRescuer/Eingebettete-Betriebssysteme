	AREA switch, CODE, readonly
		
	EXPORT ctxswitch

ctxswitch
	; R0 = &spold, R1 = spnew
	BX LR
	