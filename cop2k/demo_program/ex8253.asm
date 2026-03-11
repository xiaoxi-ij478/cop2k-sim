
CONTROL  EQU  03h
COUNT0   EQU  00h
TIMES    EQU  06h

         MOV   A, #00110110B
         WRITE CONTROL, A
         MOV   A, #TIMES
         WRITE COUNT0, A      ; low byte
         MOV   A, #0
         WRITE COUNT0, A      ; high byte
LOOP:
         NOP
         JMP   LOOP

         END

