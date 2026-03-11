    MOV  A,#00H
LOOP:
    CALL INCA
    JMP  LOOP
INCA:
    ADD  A,#01
    RET
    END
