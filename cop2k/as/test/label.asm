; defined before usage
L1:
mov a, #3
jmp L1
L2:
nop
jmp L2
; defined after usage
jmp L3
L3:
