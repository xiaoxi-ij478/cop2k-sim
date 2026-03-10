%{
#include <cstring>
#include <iostream>

#include "as.hpp"

extern int yyinstrlex(void);
extern int yyinstrlineno;
extern FILE *yyinstrin;

void yyerror(const char *s)
{
    std::cerr << "syntax error at line " << yyinstrlineno << ": " << s << std::endl;
}
%}

%union {
    unsigned char number_v;
    char *identifier_v;
    struct {
        Operand src, dst;
    } operand_v;
    InstructionYacc instruction_v;
    Signals signals_v;
    struct {
        char *name;
        bool val;
    } signal_v;
    MicroProgram microprogram_v;
    struct {
        unsigned index;
        Signals signal;
    } microprogramsignal_v;
}

%token                         OPERAND_REG_A OPERAND_MEMADDR OPERAND_REG OPERAND_IMMED OPERAND_REGADDR
%token <identifier_v>          IDENTIFIER
%token <number_v>              NUMBER
%token                         BLANK JUMP_ON_ZERO_MARKER JUMP_ON_CARRY_MARKER

%type <operand_v>              operand
%type <instruction_v>          instruction
%type <microprogram_v>         micro_program
%type <microprogramsignal_v>   micro_program_signal
%type <signals_v>              signals
%type <signal_v>               signal

%%

instructions
    : // none
    | instructions instruction {
        add_to_instruction_set($2);
        free($2.mnemonic);
        $2.mnemonic = nullptr;
    }
;

instruction
    : IDENTIFIER operand '@' NUMBER ':' micro_program ';' {
        /* special cases detection:
         * _FATCH_ MUST be at address 0x0
         * _INT_ MUST be at address 0xB8
         * DB, ORG, END, IF, ELSE and ENDIF are special instructions
         * and must not be defined
         */
        if (!strcasecmp($1, "_FATCH_") && $4 != 0x0) {
            yyerror("_FATCH_ instruction address != 0x0");
            YYERROR;
        }
        if (!strcasecmp($1, "_INT_") && $4 != 0xB8) {
            yyerror("_INT_ instruction address != 0xB8");
            YYERROR;
        }
        if (!strcasecmp($1, "DB")) {
            yyerror("DB must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "ORG")) {
            yyerror("ORG must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "END")) {
            yyerror("END must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "IF")) {
            yyerror("IF must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "ELSE")) {
            yyerror("ELSE must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "ENDIF")) {
            yyerror("ENDIF must not be defined");
            YYERROR;
        }
        if ($4 & 3) {
            yyerror("instruction address not aligned to 4 bit");
            YYERROR;
        }
        $$.byte = $4;
        $$.mnemonic = $1;
        $$.src = $2.src;
        $$.dst = $2.dst;
        $$.microprogram = $6;
    }
    | IDENTIFIER operand '@' NUMBER JUMP_ON_ZERO_MARKER ':' micro_program ';' {
        /* special cases detection:
         * this instruction's address & 0xC MUST == 1
         * _FATCH_ MUST be at address 0x0
         * _INT_ MUST be at address 0xB8
         * DB, ORG, END, IF, ELSE and ENDIF are special instructions
         * and must not be defined
         */
        if (!strcasecmp($1, "_FATCH_") && $4 != 0x0) {
            yyerror("_FATCH_ instruction address != 0x0");
            YYERROR;
        }
        if (!strcasecmp($1, "_INT_") && $4 != 0xB8) {
            yyerror("_INT_ instruction address != 0xB8");
            YYERROR;
        }
        if (!strcasecmp($1, "DB")) {
            yyerror("DB must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "ORG")) {
            yyerror("ORG must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "END")) {
            yyerror("END must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "IF")) {
            yyerror("IF must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "ELSE")) {
            yyerror("ELSE must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "ENDIF")) {
            yyerror("ENDIF must not be defined");
            YYERROR;
        }
        if ($4 & 3) {
            yyerror("instruction address not aligned to 4 bit");
            YYERROR;
        }
        if ($4 & 0xC != 1) {
            yyerror(
                "to utilize jump on zero feature, address & 0xC MUST == 1"
            );
            YYERROR;
        }
        $$.byte = $4;
        $$.mnemonic = $1;
        $$.src = $2.src;
        $$.dst = $2.dst;
        $$.microprogram = $7;
    }
    | IDENTIFIER operand '@' NUMBER JUMP_ON_CARRY_MARKER ':' micro_program ';' {
        /* special cases detection:
         * this instruction's address & 0xC MUST == 0
         * _FATCH_ MUST be at address 0x0
         * _INT_ MUST be at address 0xB8
         * DB, ORG, END, IF, ELSE and ENDIF are special instructions
         * and must not be defined
         */
        if (!strcasecmp($1, "_FATCH_") && $4 != 0x0) {
            yyerror("_FATCH_ instruction address != 0x0");
            YYERROR;
        }
        if (!strcasecmp($1, "_INT_") && $4 != 0xB8) {
            yyerror("_INT_ instruction address != 0xB8");
            YYERROR;
        }
        if (!strcasecmp($1, "DB")) {
            yyerror("DB must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "ORG")) {
            yyerror("ORG must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "END")) {
            yyerror("END must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "IF")) {
            yyerror("IF must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "ELSE")) {
            yyerror("ELSE must not be defined");
            YYERROR;
        }
        if (!strcasecmp($1, "ENDIF")) {
            yyerror("ENDIF must not be defined");
            YYERROR;
        }
        if ($4 & 3) {
            yyerror("instruction address not aligned to 4 bit");
            YYERROR;
        }
        if ($4 & 0xC != 0) {
            yyerror(
                "to utilize jump on carry feature, address & 0xC MUST == 0"
            );
            YYERROR;
        }
        $$.byte = $4;
        $$.mnemonic = $1;
        $$.src = $2.src;
        $$.dst = $2.dst;
        $$.microprogram = $7;
    }
    | error ';' {
        // error messages has been printed by their perspective rules
        yyerrok;
        yyclearin;
    }
;

operand
    : // nothing, i.e. mnemonic only
    {
        $$.src = $$.dst = Operand::NONE;
    }
    | OPERAND_REG_A {
        $$.src = Operand::REG_A;
        $$.dst = Operand::NONE;
    }
    | OPERAND_MEMADDR {
        $$.src = Operand::MEMADDR;
        $$.dst = Operand::NONE;
    }
    | OPERAND_REG {
        $$.src = Operand::REG;
        $$.dst = Operand::NONE;
    }
    | OPERAND_IMMED {
        $$.src = Operand::IMMED;
        $$.dst = Operand::NONE;
    }
    | OPERAND_REGADDR {
        $$.src = Operand::REGADDR;
        $$.dst = Operand::NONE;
    }
    | OPERAND_REG_A ',' OPERAND_REG_A {
        $$.src = Operand::REG_A;
        $$.dst = Operand::REG_A;
    }
    | OPERAND_REG_A ',' OPERAND_REG {
        $$.src = Operand::REG_A;
        $$.dst = Operand::REG;
    }
    | OPERAND_REG_A ',' OPERAND_REGADDR {
        $$.src = Operand::REG_A;
        $$.dst = Operand::REGADDR;
    }
    | OPERAND_REG_A ',' OPERAND_IMMED {
        $$.src = Operand::REG_A;
        $$.dst = Operand::IMMED;
    }
    | OPERAND_REG_A ',' OPERAND_MEMADDR {
        $$.src = Operand::REG_A;
        $$.dst = Operand::MEMADDR;
    }
    | OPERAND_REG ',' OPERAND_REG_A {
        $$.src = Operand::REG;
        $$.dst = Operand::REG_A;
    }
    | OPERAND_REG ',' OPERAND_REG {
        yyerror("xxx R?, R? cannot be constructed");
        YYERROR;
    }
    | OPERAND_REG ',' OPERAND_REGADDR {
        yyerror("xxx R?, @R? cannot be constructed");
        YYERROR;
    }
    | OPERAND_REG ',' OPERAND_IMMED {
        $$.src = Operand::REG;
        $$.dst = Operand::IMMED;
    }
    | OPERAND_REG ',' OPERAND_MEMADDR {
        $$.src = Operand::REG;
        $$.dst = Operand::MEMADDR;
    }
    | OPERAND_REGADDR ',' OPERAND_REG_A {
        $$.src = Operand::REGADDR;
        $$.dst = Operand::REG_A;
    }
    | OPERAND_REGADDR ',' OPERAND_REG {
        yyerror("xxx @R?, R? cannot be constructed");
        YYERROR;
    }
    | OPERAND_REGADDR ',' OPERAND_REGADDR {
        yyerror("xxx @R?, @R? cannot be constructed");
        YYERROR;
    }
    | OPERAND_REGADDR ',' OPERAND_IMMED {
        $$.src = Operand::REGADDR;
        $$.dst = Operand::IMMED;
    }
    | OPERAND_REGADDR ',' OPERAND_MEMADDR {
        $$.src = Operand::REGADDR;
        $$.dst = Operand::MEMADDR;
    }
    | OPERAND_IMMED ',' OPERAND_REG_A {
        $$.src = Operand::IMMED;
        $$.dst = Operand::REG_A;
    }
    | OPERAND_IMMED ',' OPERAND_REG {
        $$.src = Operand::IMMED;
        $$.dst = Operand::REG;
    }
    | OPERAND_IMMED ',' OPERAND_REGADDR {
        $$.src = Operand::IMMED;
        $$.dst = Operand::REGADDR;
    }
    | OPERAND_IMMED ',' OPERAND_IMMED {
        $$.src = Operand::IMMED;
        $$.dst = Operand::IMMED;
    }
    | OPERAND_IMMED ',' OPERAND_MEMADDR {
        $$.src = Operand::IMMED;
        $$.dst = Operand::MEMADDR;
    }
    | OPERAND_MEMADDR ',' OPERAND_REG_A {
        $$.src = Operand::MEMADDR;
        $$.dst = Operand::REG_A;
    }
    | OPERAND_MEMADDR ',' OPERAND_REG {
        $$.src = Operand::MEMADDR;
        $$.dst = Operand::REG;
    }
    | OPERAND_MEMADDR ',' OPERAND_REGADDR {
        $$.src = Operand::MEMADDR;
        $$.dst = Operand::REGADDR;
    }
    | OPERAND_MEMADDR ',' OPERAND_IMMED {
        $$.src = Operand::MEMADDR;
        $$.dst = Operand::IMMED;
    }
    | OPERAND_MEMADDR ',' OPERAND_MEMADDR {
        $$.src = Operand::MEMADDR;
        $$.dst = Operand::MEMADDR;
    }
;

micro_program
    : micro_program_signal {
        $$.signal[$1.index] = $1.signal;
    }
    | micro_program micro_program_signal {
        $$ = $1;
        $$.signal[$2.index] = $2.signal;
    }
;

micro_program_signal
    : NUMBER ':' signals {
        if ($1 > 3) {
            yyerror("micro program index > 3");
            YYERROR;
        }

        $$.index = $1;
        $$.signal = $3;
    }
;

signals
    : signal {
        std::memset(&$$, 1, sizeof($$));

        if (!strcasecmp($1.name, "emwr"))
            $$.emwr = $1.val;

        else if (!strcasecmp($1.name, "emrd"))
            $$.emrd = $1.val;

        else if (!strcasecmp($1.name, "pcoe"))
            $$.pcoe = $1.val;

        else if (!strcasecmp($1.name, "emen"))
            $$.emen = $1.val;

        else if (!strcasecmp($1.name, "iren"))
            $$.iren = $1.val;

        else if (!strcasecmp($1.name, "eint"))
            $$.eint = $1.val;

        else if (!strcasecmp($1.name, "elp"))
            $$.elp = $1.val;

        else if (!strcasecmp($1.name, "maren"))
            $$.maren = $1.val;

        else if (!strcasecmp($1.name, "maroe"))
            $$.maroe = $1.val;

        else if (!strcasecmp($1.name, "outen"))
            $$.outen = $1.val;

        else if (!strcasecmp($1.name, "sten"))
            $$.sten = $1.val;

        else if (!strcasecmp($1.name, "rrd"))
            $$.rrd = $1.val;

        else if (!strcasecmp($1.name, "rwr"))
            $$.rwr = $1.val;

        else if (!strcasecmp($1.name, "cn"))
            $$.cn = $1.val;

        else if (!strcasecmp($1.name, "fen"))
            $$.fen = $1.val;

        else if (!strcasecmp($1.name, "x2"))
            $$.x2 = $1.val;

        else if (!strcasecmp($1.name, "x1"))
            $$.x1 = $1.val;

        else if (!strcasecmp($1.name, "x0"))
            $$.x0 = $1.val;

        else if (!strcasecmp($1.name, "wen"))
            $$.wen = $1.val;

        else if (!strcasecmp($1.name, "aen"))
            $$.aen = $1.val;

        else if (!strcasecmp($1.name, "s2"))
            $$.s2 = $1.val;

        else if (!strcasecmp($1.name, "s1"))
            $$.s1 = $1.val;

        else if (!strcasecmp($1.name, "s0"))
            $$.s0 = $1.val;

        else {
            yyerror("Invalid signal name");
            YYERROR;
        }

        free($1.name);
        $1.name = nullptr;
    }
    | signals signal {
        $$ = $1;

        if (!strcasecmp($2.name, "emwr"))
            $$.emwr = $2.val;

        else if (!strcasecmp($2.name, "emrd"))
            $$.emrd = $2.val;

        else if (!strcasecmp($2.name, "pcoe"))
            $$.pcoe = $2.val;

        else if (!strcasecmp($2.name, "emen"))
            $$.emen = $2.val;

        else if (!strcasecmp($2.name, "iren"))
            $$.iren = $2.val;

        else if (!strcasecmp($2.name, "eint"))
            $$.eint = $2.val;

        else if (!strcasecmp($2.name, "elp"))
            $$.elp = $2.val;

        else if (!strcasecmp($2.name, "maren"))
            $$.maren = $2.val;

        else if (!strcasecmp($2.name, "maroe"))
            $$.maroe = $2.val;

        else if (!strcasecmp($2.name, "outen"))
            $$.outen = $2.val;

        else if (!strcasecmp($2.name, "sten"))
            $$.sten = $2.val;

        else if (!strcasecmp($2.name, "rrd"))
            $$.rrd = $2.val;

        else if (!strcasecmp($2.name, "rwr"))
            $$.rwr = $2.val;

        else if (!strcasecmp($2.name, "cn"))
            $$.cn = $2.val;

        else if (!strcasecmp($2.name, "fen"))
            $$.fen = $2.val;

        else if (!strcasecmp($2.name, "x2"))
            $$.x2 = $2.val;

        else if (!strcasecmp($2.name, "x1"))
            $$.x1 = $2.val;

        else if (!strcasecmp($2.name, "x0"))
            $$.x0 = $2.val;

        else if (!strcasecmp($2.name, "wen"))
            $$.wen = $2.val;

        else if (!strcasecmp($2.name, "aen"))
            $$.aen = $2.val;

        else if (!strcasecmp($2.name, "s2"))
            $$.s2 = $2.val;

        else if (!strcasecmp($2.name, "s1"))
            $$.s1 = $2.val;

        else if (!strcasecmp($2.name, "s0"))
            $$.s0 = $2.val;

        else {
            yyerror("Invalid signal name");
            YYERROR;
        }

        free($2.name);
        $2.name = nullptr;
    }
;

signal
    : IDENTIFIER {
        $$.name = $1;
        $$.val = true;
    }
    | '!' IDENTIFIER {
        $$.name = $2;
        $$.val = false;
    }
;

%%

void parse_instruction_file(FILE *in)
{
    yyinstrin = in;
    if (yyparse())
        throw ParseFailure("failed to parse file");
}
