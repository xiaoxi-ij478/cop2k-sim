    MOV  A,#01
LOOP:
    SUB  A,#01
    JC   LOOP
    JZ   LOOP
    JMP  0
    END
