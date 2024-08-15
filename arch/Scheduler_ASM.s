; G8RTOS_SchedulerASM.s
; Holds all ASM functions needed for the scheduler
; Note: If you have an h file, do not have a C file and an S file of the same name

	; Functions Defined
	.def start_RTOS, PendSV_Handler

	; Dependencies
	.ref currentThread, G8RTOS_Scheduler

	.thumb		; Set to thumb mode
	.align 2	; Align by 2 bytes (thumb mode uses allignment by 2 or 4)
	.text		; Text section

; Need to have the address defined in file 
; (label needs to be close enough to asm code to be reached with PC relative addressing)
RunningPtr: .field currentThread, 32

; G8RTOS_Start
;	Sets the first thread to be the currently running thread
;	Starts the currently running thread by setting Link Register to tcb's Program Counter
start_RTOS:

	.asmfunc

	;load the stack pointer for the first thread
	LDR r5, RunningPtr ;loads tcb double pointer into r5
	LDR r5, [r5] ;loads tcb pointer into r5
	LDR sp, [r5] ; load stack pointer from new tcb

	;load initial context, see addThread for format
	pop {r0-r12}
	pop {r14} ;link register
	pop {r0} ;ignore pc, we will set it when we branch
	pop {xpsr} ;status register, with thumb bit set

	;enable interrupts

	BX LR ;brach to LR, lets get started

	.endasmfunc

; PendSV_Handler
; - Performs a context switch in G8RTOS
; 	- Saves remaining registers into thread stack
;	- Saves current stack pointer to tcb
;	- Calls G8RTOS_Scheduler to get new tcb
;	- Set stack pointer to new stack pointer from new tcb
;	- Pops registers from thread stack
PendSV_Handler:
	
	.asmfunc

	;store rest of context, pendSV did most of it
	push {r4-r11}

	;store stack pointer to currently running thread
	LDR r5, RunningPtr ;loads tcb double pointer into r5
	LDR r5, [r5] ;loads tcb pointer into r5
	STR sp, [r5] ;store stack pointer to tcb struct's SP


	MOV r4, lr ;save lr
	;C call to get next thread from scheduler
	BL G8RTOS_Scheduler
	MOV lr, r4 ;restore lr

    ;load the stack pointer from the currently running thread
	LDR r5, RunningPtr ;loads tcb double pointer into r5
	LDR r5, [r5] ;loads tcb pointer into r5
	LDR sp, [r5] ; load stack pointer from new tcb

    ;pop registers, ie load context
	pop {r4-r11}
    ;pendSV will handle the rest

    ;start next process back up
    BX LR

	.endasmfunc
	
	; end of the asm file
	.align
	.end
