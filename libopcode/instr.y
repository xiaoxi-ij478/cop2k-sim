%{
#include <iostream>
#include <functional>

#include "libopcode.hpp"
#include "libopcode_yacc.hpp"

extern int yyinstrlex(void);
extern int yyinstrlineno;
extern void yyinstrset_lineno(int);
extern void yyinstrset_in(FILE *);
extern FILE *yyinstrin;

static COP2K::Opcode *current_opcode = nullptr;

void yyerror(const char *s)
{
    std::cerr << "syntax error at line " << yyinstrlineno << ": " << s << std::endl;
}
%}

%union {
    unsigned char number_v;
    char *identifier_v;
    struct {
        COP2K::Operand src, dst;
    } operand_v;
    struct {
        unsigned char byte;
        char *mnemonic;
        COP2K::Operand src, dst;
        struct MicroProgram microprogram;
    } instruction_v;
    struct Signals signals_v;
    struct {
        char *name;
        bool val;
    } signal_v;
    struct {
        unsigned index;
        struct Signals signal;
    } microprogramsignal_v;
    struct MicroProgram microprogram_v;
}

%token                         OPERAND_REG_A OPERAND_MEMADDR OPERAND_REG OPERAND_IMMED OPERAND_REGADDR
%token <identifier_v>          IDENTIFIER
%token <number_v>              NUMBER
%token                         JUMP_ON_ZERO_MARKER JUMP_ON_CARRY_MARKER

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
        std::array<std::bitset<24>, 4> arr;
        for (unsigned char i = 0; i < 4; i++)
            arr.at(i) = $2.microprogram.signals[i].to_bitset();

        current_opcode->add($2.byte, $2.mnemonic, "", $2.src, $2.dst, arr);
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
        if ($4 == 0x0 && strcasecmp($1, "_FATCH_")) {
            yyerror("instruction @ 0x0 MUST be _FATCH_");
            YYERROR;
        }
        if (!strcasecmp($1, "_INT_") && $4 != 0xB8) {
            yyerror("_INT_ instruction address != 0xB8");
            YYERROR;
        }
        if ($4 == 0xB8 && strcasecmp($1, "_INT_")) {
            yyerror("instruction @ 0xB8 MUST be _INT_");
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

        for (unsigned char i = 0; i < $6.signal_count; i++)
            if (!$6.signals[i].elp)
                if (($4 & 0x8) >> 3 != 1) {
                    yyerror("to make sure jump unconditionally, (address & 0x8) >> 3 MUST == 1");
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
         * this instruction's (address & 0xC) >> 2 MUST == 1
         * _FATCH_ MUST be at address 0x0
         * _INT_ MUST be at address 0xB8
         * DB, ORG, END, IF, ELSE and ENDIF are special instructions
         * and must not be defined
         */
        if (!strcasecmp($1, "_FATCH_") && $4 != 0x0) {
            yyerror("_FATCH_ instruction address != 0x0");
            YYERROR;
        }
        if ($4 == 0x0 && strcasecmp($1, "_FATCH_")) {
            yyerror("instruction @ 0x0 MUST be _FATCH_");
            YYERROR;
        }
        if (!strcasecmp($1, "_INT_") && $4 != 0xB8) {
            yyerror("_INT_ instruction address != 0xB8");
            YYERROR;
        }
        if ($4 == 0xB8 && strcasecmp($1, "_INT_")) {
            yyerror("instruction @ 0xB8 MUST be _INT_");
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
        if (($4 & 0xC) >> 2 != 1) {
            yyerror("to utilize jump on zero feature, (address & 0xC) >> 2 MUST == 1");
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
         * this instruction's (address & 0xC) >> 2 MUST == 0
         * _FATCH_ MUST be at address 0x0
         * _INT_ MUST be at address 0xB8
         * DB, ORG, END, IF, ELSE and ENDIF are special instructions
         * and must not be defined
         */
        if (!strcasecmp($1, "_FATCH_") && $4 != 0x0) {
            yyerror("_FATCH_ instruction address != 0x0");
            YYERROR;
        }
        if ($4 == 0x0 && strcasecmp($1, "_FATCH_")) {
            yyerror("instruction @ 0x0 MUST be _FATCH_");
            YYERROR;
        }
        if (!strcasecmp($1, "_INT_") && $4 != 0xB8) {
            yyerror("_INT_ instruction address != 0xB8");
            YYERROR;
        }
        if ($4 == 0xB8 && strcasecmp($1, "_INT_")) {
            yyerror("instruction @ 0xB8 MUST be _INT_");
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
        if (($4 & 0xC) >> 2 != 0) {
            yyerror("to utilize jump on carry feature, (address & 0xC) >> 2 MUST == 0");
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
    : { // nothing, i.e. mnemonic only
        $$.src = $$.dst = COP2K::Operand::NONE;
    }
    | OPERAND_REG_A {
        $$.src = COP2K::Operand::REG_A;
        $$.dst = COP2K::Operand::NONE;
    }
    | OPERAND_MEMADDR {
        $$.src = COP2K::Operand::MEMADDR;
        $$.dst = COP2K::Operand::NONE;
    }
    | OPERAND_REG {
        $$.src = COP2K::Operand::REG;
        $$.dst = COP2K::Operand::NONE;
    }
    | OPERAND_IMMED {
        $$.src = COP2K::Operand::IMMED;
        $$.dst = COP2K::Operand::NONE;
    }
    | OPERAND_REGADDR {
        $$.src = COP2K::Operand::REGADDR;
        $$.dst = COP2K::Operand::NONE;
    }
    | OPERAND_REG_A ',' OPERAND_REG_A {
        $$.src = COP2K::Operand::REG_A;
        $$.dst = COP2K::Operand::REG_A;
    }
    | OPERAND_REG_A ',' OPERAND_REG {
        $$.src = COP2K::Operand::REG_A;
        $$.dst = COP2K::Operand::REG;
    }
    | OPERAND_REG_A ',' OPERAND_REGADDR {
        $$.src = COP2K::Operand::REG_A;
        $$.dst = COP2K::Operand::REGADDR;
    }
    | OPERAND_REG_A ',' OPERAND_IMMED {
        $$.src = COP2K::Operand::REG_A;
        $$.dst = COP2K::Operand::IMMED;
    }
    | OPERAND_REG_A ',' OPERAND_MEMADDR {
        $$.src = COP2K::Operand::REG_A;
        $$.dst = COP2K::Operand::MEMADDR;
    }
    | OPERAND_REG ',' OPERAND_REG_A {
        $$.src = COP2K::Operand::REG;
        $$.dst = COP2K::Operand::REG_A;
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
        $$.src = COP2K::Operand::REG;
        $$.dst = COP2K::Operand::IMMED;
    }
    | OPERAND_REG ',' OPERAND_MEMADDR {
        $$.src = COP2K::Operand::REG;
        $$.dst = COP2K::Operand::MEMADDR;
    }
    | OPERAND_REGADDR ',' OPERAND_REG_A {
        $$.src = COP2K::Operand::REGADDR;
        $$.dst = COP2K::Operand::REG_A;
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
        $$.src = COP2K::Operand::REGADDR;
        $$.dst = COP2K::Operand::IMMED;
    }
    | OPERAND_REGADDR ',' OPERAND_MEMADDR {
        $$.src = COP2K::Operand::REGADDR;
        $$.dst = COP2K::Operand::MEMADDR;
    }
    | OPERAND_IMMED ',' OPERAND_REG_A {
        $$.src = COP2K::Operand::IMMED;
        $$.dst = COP2K::Operand::REG_A;
    }
    | OPERAND_IMMED ',' OPERAND_REG {
        $$.src = COP2K::Operand::IMMED;
        $$.dst = COP2K::Operand::REG;
    }
    | OPERAND_IMMED ',' OPERAND_REGADDR {
        $$.src = COP2K::Operand::IMMED;
        $$.dst = COP2K::Operand::REGADDR;
    }
    | OPERAND_IMMED ',' OPERAND_IMMED {
        $$.src = COP2K::Operand::IMMED;
        $$.dst = COP2K::Operand::IMMED;
    }
    | OPERAND_IMMED ',' OPERAND_MEMADDR {
        $$.src = COP2K::Operand::IMMED;
        $$.dst = COP2K::Operand::MEMADDR;
    }
    | OPERAND_MEMADDR ',' OPERAND_REG_A {
        $$.src = COP2K::Operand::MEMADDR;
        $$.dst = COP2K::Operand::REG_A;
    }
    | OPERAND_MEMADDR ',' OPERAND_REG {
        $$.src = COP2K::Operand::MEMADDR;
        $$.dst = COP2K::Operand::REG;
    }
    | OPERAND_MEMADDR ',' OPERAND_REGADDR {
        $$.src = COP2K::Operand::MEMADDR;
        $$.dst = COP2K::Operand::REGADDR;
    }
    | OPERAND_MEMADDR ',' OPERAND_IMMED {
        $$.src = COP2K::Operand::MEMADDR;
        $$.dst = COP2K::Operand::IMMED;
    }
    | OPERAND_MEMADDR ',' OPERAND_MEMADDR {
        $$.src = COP2K::Operand::MEMADDR;
        $$.dst = COP2K::Operand::MEMADDR;
    }
;

micro_program
    : micro_program_signal {
        $$.clear();

        if (!$1.signal.empty()) {
            if ($$.signal_count++ != $1.index) {
                yyerror("micro program index not continous");
                YYERROR;
            }

            $$.signals[$1.index] = $1.signal;
        }
    }
    | micro_program micro_program_signal {
        $$ = $1;

        if (!$2.signal.empty()) {
            if ($$.signal_count++ != $2.index) {
                yyerror("micro program index not continous");
                YYERROR;
            }

            $$.signals[$2.index] = $2.signal;
        }
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
        $$.clear();

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

void COP2K::parse_instruction_file(FILE *in, COP2K::Opcode *opcode)
{
    yyinstrset_lineno(1);
    yyinstrset_in(in);
    current_opcode = opcode;

    int result = yyparse();

    current_opcode = nullptr;

    if (result)
        throw std::runtime_error("failed to parse file");
}
