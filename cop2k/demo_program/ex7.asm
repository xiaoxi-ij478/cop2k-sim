

      LD     A, #12H
      STA    R0
      LDW    R0
      ADDW   A
      STA    R1
      ST     A, 20H
      JMP    0
      END

