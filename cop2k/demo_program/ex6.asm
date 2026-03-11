    MOV  A, #00H
LOOP:
    OUT
    JMP  LOOP

    ORG  0E0H
    ADD  A, #01
    RETI
    END
