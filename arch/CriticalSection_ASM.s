; WARNING This is almost certainly non-functional. It's a skeleton of the old msp432 code. 
; TODO Replace it with something that actually works.

.text

; Starts a critical section
; 	- Saves the state of the current PRIMASK (I-bit)
; 	- Disables interrupts
; Returns: The current PRIMASK State
StartCriticalSection:
	MRS R0, PRIMASK		; Save PRIMASK to R0 (Return Register)
	CPSID I				; Disable Interrupts
	BX LR				; Return

; Ends a critical Section
; 	- Restores the state of the PRIMASK given an input
; Param R0: PRIMASK State to update
EndCriticalSection:
	MSR PRIMASK, R0		; Save R0 (Param) to PRIMASK
	BX LR				; Return

