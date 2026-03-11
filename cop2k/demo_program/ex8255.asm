
      CONTROL EQU 03H
      PORTA   EQU 00H
      PORTB   EQU 01H

      MOV    A, #90H
      WRITE  CONTROL, A
LOOP:
      READ   A, PORTA
      CPL    A
      WRITE  PORTB, A
      JMP    LOOP

      END
