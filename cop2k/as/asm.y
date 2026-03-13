%{
#include <iostream>
#include <stack>
#include <cstring>
#include <functional>

#include "as.hpp"

extern int yyasmlex(void);
extern int yyasmlineno;
extern FILE *yyasmin;

static std::stack<bool> block_status;
static COP2K::AS *current_as = nullptr;

static bool block_active() { return block_status.empty() || block_status.top(); }
static void push_block(bool val) { block_status.push(val); }
static void pop_block() { block_status.pop(); }
static bool top_block() { return block_status.top(); }
static bool no_block() { return block_status.empty(); }

#define RN_RN_NOT_SUPPORTED(a, b) \
    do { \
        yyerror("R"#a", R"#b" not supported"); \
        YYERROR; \
    } while(0)

#define RN_ARN_NOT_SUPPORTED(a, b) \
    do { \
        yyerror("R"#a", @R"#b" not supported"); \
        YYERROR; \
    } while(0)

#define ARN_RN_NOT_SUPPORTED(a, b) \
    do { \
        yyerror("@R"#a", R"#b" not supported"); \
        YYERROR; \
    } while(0)

#define ARN_ARN_NOT_SUPPORTED(a, b) \
    do { \
        yyerror("@R"#a", @R"#b" not supported"); \
        YYERROR; \
    } while(0)

void yyerror(const char *s)
{
    std::cerr << "syntax error at line " << yyasmlineno << ": " << s << std::endl;
}
%}

%union {
    unsigned char number_v;
    char *identifier_v;
    struct {
        char *mnemonic;
        char *label;
        struct InstructionOperand operand;
    } instruction_v;
    struct InstructionOperand instruction_operand_v;
}

%token EQU R0 R1 R2 R3 AT_R0 AT_R1 AT_R2 AT_R3
%token DB ORG END IF ELSE ENDIF

%token <identifier_v>          IDENTIFIER
%token <number_v>              NUMBER

%type  <number_v>              expression
%type  <instruction_v>         instruction
%type  <instruction_operand_v> operand

// operator precedence is the same as that in C
%left '|'
%left '^'
%left '&'
%left '<' '>'
%left '+' '-'
%left '*' '/' '%'
%right '!' UNNEG UNPOS

%%

program
    : // none
    | program instruction {
        if (block_active())
            current_as->add_instruction($2.mnemonic, $2.label, $2.operand);

        free($2.mnemonic);
        $2.mnemonic = nullptr;

        if ($2.label) {
            free($2.label);
            $2.label = nullptr;
        }
    }
;

instruction
    : IDENTIFIER operand '\n'
       /* we must not use fixed mnemonic set
        * because instruction set is not fixed */
    {
        $$.mnemonic = $1;
        $$.label = nullptr;
        $$.operand = $2;
    }
    | IDENTIFIER ':' '\n' {
        $$.mnemonic = strdup("00label");
        $$.label = $1;
        $$.operand.src_type = $$.operand.dst_type = COP2K::Operand::NONE;
    }
    | IDENTIFIER EQU expression '\n' {
        $$.mnemonic = strdup("00const");
        $$.label = $1;
        $$.operand.src_type = COP2K::Operand::IMMED;
        $$.operand.dst_type = COP2K::Operand::NONE;
        $$.operand.src = $3;
    }
    | DB expression '\n' {
        $$.mnemonic = strdup("db");
        $$.label = nullptr;
        $$.operand.src_type = COP2K::Operand::MEMADDR;
        $$.operand.dst_type = COP2K::Operand::NONE;
        $$.operand.src = $2;
    }
    | ORG expression '\n' {
        $$.mnemonic = strdup("org");
        $$.label = nullptr;
        $$.operand.src_type = COP2K::Operand::MEMADDR;
        $$.operand.dst_type = COP2K::Operand::NONE;
        $$.operand.src = $2;
    }
    | END '\n' {
        $$.mnemonic = strdup("end");
        $$.label = nullptr;
        $$.operand.src_type = $$.operand.dst_type = COP2K::Operand::NONE;
    }
    | IF expression '\n' {
        $$.mnemonic = strdup("if");
        $$.label = nullptr;
        $$.operand.src_type = COP2K::Operand::IMMED;
        $$.operand.dst_type = COP2K::Operand::NONE;
        $$.operand.src = $2;

        push_block($2);
    }
    | ELSE '\n' {
        $$.mnemonic = strdup("else");
        $$.label = nullptr;
        $$.operand.src_type = $$.operand.dst_type = COP2K::Operand::NONE;

        if (no_block()) {
            free($$.mnemonic);
            $$.mnemonic = nullptr;

            yyerror("'else' with no corresponding 'if'");
            YYABORT;
        }

        bool n = !top_block();
        pop_block();
        push_block(n);
    }
    | ENDIF '\n' {
        $$.mnemonic = strdup("endif");
        $$.label = nullptr;
        $$.operand.src_type = $$.operand.dst_type = COP2K::Operand::NONE;

        if (no_block()) {
            free($$.mnemonic);
            $$.mnemonic = nullptr;

            yyerror("'endif' with no corresponding 'if'");
            YYABORT;
        }

        pop_block();
    }
    | error '\n' {
        // error messages has been printed by their perspective rules
        yyerrok;
        yyclearin;
    }
;

operand
    : { // none
        $$.src_type = $$.dst_type = COP2K::Operand::NONE;
    }
    | 'a' {
        $$.src_type = COP2K::Operand::REG_A;
        $$.dst_type = COP2K::Operand::NONE;
    }
    | R0 {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::NONE;
        $$.src = 0;
    }
    | R1 {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::NONE;
        $$.src = 1;
    }
    | R2 {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::NONE;
        $$.src = 2;
    }
    | R3 {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::NONE;
        $$.src = 3;
    }
    | AT_R0 {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::NONE;
        $$.src = 0;
    }
    | AT_R1 {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::NONE;
        $$.src = 1;
    }
    | AT_R2 {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::NONE;
        $$.src = 2;
    }
    | AT_R3 {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::NONE;
        $$.src = 3;
    }
    | '#' expression {
        $$.src_type = COP2K::Operand::IMMED;
        $$.dst_type = COP2K::Operand::NONE;
        $$.src = $2;
    }
    | expression {
        $$.src_type = COP2K::Operand::MEMADDR;
        $$.dst_type = COP2K::Operand::NONE;
        $$.src = $1;
    }
    | 'a' ',' 'a' {
        $$.src_type = COP2K::Operand::REG_A;
        $$.dst_type = COP2K::Operand::REG_A;
    }
    | 'a' ',' R0 {
        $$.src_type = COP2K::Operand::REG_A;
        $$.dst_type = COP2K::Operand::REG;
        $$.dst = 0;
    }
    | 'a' ',' R1 {
        $$.src_type = COP2K::Operand::REG_A;
        $$.dst_type = COP2K::Operand::REG;
        $$.dst = 1;
    }
    | 'a' ',' R2 {
        $$.src_type = COP2K::Operand::REG_A;
        $$.dst_type = COP2K::Operand::REG;
        $$.dst = 2;
    }
    | 'a' ',' R3 {
        $$.src_type = COP2K::Operand::REG_A;
        $$.dst_type = COP2K::Operand::REG;
        $$.dst = 3;
    }
    | 'a' ',' AT_R0 {
        $$.src_type = COP2K::Operand::REG_A;
        $$.dst_type = COP2K::Operand::REGADDR;
        $$.dst = 0;
    }
    | 'a' ',' AT_R1 {
        $$.src_type = COP2K::Operand::REG_A;
        $$.dst_type = COP2K::Operand::REGADDR;
        $$.dst = 1;
    }
    | 'a' ',' AT_R2 {
        $$.src_type = COP2K::Operand::REG_A;
        $$.dst_type = COP2K::Operand::REGADDR;
        $$.dst = 2;
    }
    | 'a' ',' AT_R3 {
        $$.src_type = COP2K::Operand::REG_A;
        $$.dst_type = COP2K::Operand::REGADDR;
        $$.dst = 3;
    }
    | 'a' ',' '#' expression {
        $$.src_type = COP2K::Operand::REG_A;
        $$.dst_type = COP2K::Operand::IMMED;
        $$.dst = $4;
    }
    | 'a' ',' expression {
        $$.src_type = COP2K::Operand::REG_A;
        $$.dst_type = COP2K::Operand::MEMADDR;
        $$.dst = $3;
    }
    | R0 ',' 'a' {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::REG_A;
        $$.src = 0;
    }
    | R0 ',' R0     { RN_RN_NOT_SUPPORTED(0, 0); }
    | R0 ',' R1     { RN_RN_NOT_SUPPORTED(0, 1); }
    | R0 ',' R2     { RN_RN_NOT_SUPPORTED(0, 2); }
    | R0 ',' R3     { RN_RN_NOT_SUPPORTED(0, 3); }
    | R0 ',' AT_R0  { RN_ARN_NOT_SUPPORTED(0, 0); }
    | R0 ',' AT_R1  { RN_ARN_NOT_SUPPORTED(0, 1); }
    | R0 ',' AT_R2  { RN_ARN_NOT_SUPPORTED(0, 2); }
    | R0 ',' AT_R3  { RN_ARN_NOT_SUPPORTED(0, 3); }
    | R0 ',' '#' expression {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::IMMED;
        $$.src = 0;
        $$.dst = $4;
    }
    | R0 ',' expression {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::MEMADDR;
        $$.src = 0;
        $$.dst = $3;
    }
    | R1 ',' 'a' {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::REG_A;
        $$.src = 1;
    }
    | R1 ',' R0     { RN_RN_NOT_SUPPORTED(1, 0); }
    | R1 ',' R1     { RN_RN_NOT_SUPPORTED(1, 1); }
    | R1 ',' R2     { RN_RN_NOT_SUPPORTED(1, 2); }
    | R1 ',' R3     { RN_RN_NOT_SUPPORTED(1, 3); }
    | R1 ',' AT_R0  { RN_ARN_NOT_SUPPORTED(1, 0); }
    | R1 ',' AT_R1  { RN_ARN_NOT_SUPPORTED(1, 1); }
    | R1 ',' AT_R2  { RN_ARN_NOT_SUPPORTED(1, 2); }
    | R1 ',' AT_R3  { RN_ARN_NOT_SUPPORTED(1, 3); }
    | R1 ',' '#' expression {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::IMMED;
        $$.src = 1;
        $$.dst = $4;
    }
    | R1 ',' expression {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::MEMADDR;
        $$.src = 1;
        $$.dst = $3;
    }
    | R2 ',' 'a' {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::REG_A;
        $$.src = 2;
    }
    | R2 ',' R0     { RN_RN_NOT_SUPPORTED(2, 0); }
    | R2 ',' R1     { RN_RN_NOT_SUPPORTED(2, 1); }
    | R2 ',' R2     { RN_RN_NOT_SUPPORTED(2, 2); }
    | R2 ',' R3     { RN_RN_NOT_SUPPORTED(2, 3); }
    | R2 ',' AT_R0  { RN_ARN_NOT_SUPPORTED(2, 0); }
    | R2 ',' AT_R1  { RN_ARN_NOT_SUPPORTED(2, 1); }
    | R2 ',' AT_R2  { RN_ARN_NOT_SUPPORTED(2, 2); }
    | R2 ',' AT_R3  { RN_ARN_NOT_SUPPORTED(2, 3); }
    | R2 ',' '#' expression {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::IMMED;
        $$.src = 2;
        $$.dst = $4;
    }
    | R2 ',' expression {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::MEMADDR;
        $$.src = 2;
        $$.dst = $3;
    }
    | R3 ',' 'a' {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::REG_A;
        $$.src = 3;
    }
    | R3 ',' R0     { RN_RN_NOT_SUPPORTED(3, 0); }
    | R3 ',' R1     { RN_RN_NOT_SUPPORTED(3, 1); }
    | R3 ',' R2     { RN_RN_NOT_SUPPORTED(3, 2); }
    | R3 ',' R3     { RN_RN_NOT_SUPPORTED(3, 3); }
    | R3 ',' AT_R0  { RN_ARN_NOT_SUPPORTED(3, 0); }
    | R3 ',' AT_R1  { RN_ARN_NOT_SUPPORTED(3, 1); }
    | R3 ',' AT_R2  { RN_ARN_NOT_SUPPORTED(3, 2); }
    | R3 ',' AT_R3  { RN_ARN_NOT_SUPPORTED(3, 3); }
    | R3 ',' '#' expression {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::IMMED;
        $$.src = 3;
        $$.dst = $4;
    }
    | R3 ',' expression {
        $$.src_type = COP2K::Operand::REG;
        $$.dst_type = COP2K::Operand::MEMADDR;
        $$.src = 3;
        $$.dst = $3;
    }
    | AT_R0 ',' 'a' {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::REG_A;
        $$.src = 0;
    }
    | AT_R0 ',' R0     { ARN_RN_NOT_SUPPORTED(0, 0); }
    | AT_R0 ',' R1     { ARN_RN_NOT_SUPPORTED(0, 1); }
    | AT_R0 ',' R2     { ARN_RN_NOT_SUPPORTED(0, 2); }
    | AT_R0 ',' R3     { ARN_RN_NOT_SUPPORTED(0, 3); }
    | AT_R0 ',' AT_R0  { ARN_ARN_NOT_SUPPORTED(0, 0); }
    | AT_R0 ',' AT_R1  { ARN_ARN_NOT_SUPPORTED(0, 1); }
    | AT_R0 ',' AT_R2  { ARN_ARN_NOT_SUPPORTED(0, 2); }
    | AT_R0 ',' AT_R3  { ARN_ARN_NOT_SUPPORTED(0, 3); }
    | AT_R0 ',' '#' expression {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::IMMED;
        $$.src = 0;
        $$.dst = $4;
    }
    | AT_R0 ',' expression {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::MEMADDR;
        $$.src = 0;
        $$.dst = $3;
    }
    | AT_R1 ',' 'a' {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::REG_A;
        $$.src = 1;
    }
    | AT_R1 ',' R0     { ARN_RN_NOT_SUPPORTED(1, 0); }
    | AT_R1 ',' R1     { ARN_RN_NOT_SUPPORTED(1, 1); }
    | AT_R1 ',' R2     { ARN_RN_NOT_SUPPORTED(1, 2); }
    | AT_R1 ',' R3     { ARN_RN_NOT_SUPPORTED(1, 3); }
    | AT_R1 ',' AT_R0  { ARN_ARN_NOT_SUPPORTED(1, 0); }
    | AT_R1 ',' AT_R1  { ARN_ARN_NOT_SUPPORTED(1, 1); }
    | AT_R1 ',' AT_R2  { ARN_ARN_NOT_SUPPORTED(1, 2); }
    | AT_R1 ',' AT_R3  { ARN_ARN_NOT_SUPPORTED(1, 3); }
    | AT_R1 ',' '#' expression {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::IMMED;
        $$.src = 1;
        $$.dst = $4;
    }
    | AT_R1 ',' expression {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::MEMADDR;
        $$.src = 1;
        $$.dst = $3;
    }
    | AT_R2 ',' 'a' {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::REG_A;
        $$.src = 2;
    }
    | AT_R2 ',' R0     { ARN_RN_NOT_SUPPORTED(2, 0); }
    | AT_R2 ',' R1     { ARN_RN_NOT_SUPPORTED(2, 1); }
    | AT_R2 ',' R2     { ARN_RN_NOT_SUPPORTED(2, 2); }
    | AT_R2 ',' R3     { ARN_RN_NOT_SUPPORTED(2, 3); }
    | AT_R2 ',' AT_R0  { ARN_ARN_NOT_SUPPORTED(2, 0); }
    | AT_R2 ',' AT_R1  { ARN_ARN_NOT_SUPPORTED(2, 1); }
    | AT_R2 ',' AT_R2  { ARN_ARN_NOT_SUPPORTED(2, 2); }
    | AT_R2 ',' AT_R3  { ARN_ARN_NOT_SUPPORTED(2, 3); }
    | AT_R2 ',' '#' expression {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::IMMED;
        $$.src = 2;
        $$.dst = $4;
    }
    | AT_R2 ',' expression {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::MEMADDR;
        $$.src = 2;
        $$.dst = $3;
    }
    | AT_R3 ',' 'a' {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::REG_A;
        $$.src = 3;
    }
    | AT_R3 ',' R0     { ARN_RN_NOT_SUPPORTED(3, 0); }
    | AT_R3 ',' R1     { ARN_RN_NOT_SUPPORTED(3, 1); }
    | AT_R3 ',' R2     { ARN_RN_NOT_SUPPORTED(3, 2); }
    | AT_R3 ',' R3     { ARN_RN_NOT_SUPPORTED(3, 3); }
    | AT_R3 ',' AT_R0  { ARN_ARN_NOT_SUPPORTED(3, 0); }
    | AT_R3 ',' AT_R1  { ARN_ARN_NOT_SUPPORTED(3, 1); }
    | AT_R3 ',' AT_R2  { ARN_ARN_NOT_SUPPORTED(3, 2); }
    | AT_R3 ',' AT_R3  { ARN_ARN_NOT_SUPPORTED(3, 3); }
    | AT_R3 ',' '#' expression {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::IMMED;
        $$.src = 3;
        $$.dst = $4;
    }
    | AT_R3 ',' expression {
        $$.src_type = COP2K::Operand::REGADDR;
        $$.dst_type = COP2K::Operand::MEMADDR;
        $$.src = 3;
        $$.dst = $3;
    }
    | '#' expression ',' 'a' {
        $$.src_type = COP2K::Operand::IMMED;
        $$.dst_type = COP2K::Operand::REG_A;
        $$.src = $2;
    }
    | '#' expression ',' R0 {
        $$.src_type = COP2K::Operand::IMMED;
        $$.dst_type = COP2K::Operand::REG;
        $$.src = $2;
        $$.dst = 0;
    }
    | '#' expression ',' R1 {
        $$.src_type = COP2K::Operand::IMMED;
        $$.dst_type = COP2K::Operand::REG;
        $$.src = $2;
        $$.dst = 1;
    }
    | '#' expression ',' R2 {
        $$.src_type = COP2K::Operand::IMMED;
        $$.dst_type = COP2K::Operand::REG;
        $$.src = $2;
        $$.dst = 2;
    }
    | '#' expression ',' R3 {
        $$.src_type = COP2K::Operand::IMMED;
        $$.dst_type = COP2K::Operand::REG;
        $$.src = $2;
        $$.dst = 3;
    }
    | '#' expression ',' AT_R0 {
        $$.src_type = COP2K::Operand::IMMED;
        $$.dst_type = COP2K::Operand::REGADDR;
        $$.src = $2;
        $$.dst = 0;
    }
    | '#' expression ',' AT_R1 {
        $$.src_type = COP2K::Operand::IMMED;
        $$.dst_type = COP2K::Operand::REGADDR;
        $$.src = $2;
        $$.dst = 1;
    }
    | '#' expression ',' AT_R2 {
        $$.src_type = COP2K::Operand::IMMED;
        $$.dst_type = COP2K::Operand::REGADDR;
        $$.src = $2;
        $$.dst = 2;
    }
    | '#' expression ',' AT_R3 {
        $$.src_type = COP2K::Operand::IMMED;
        $$.dst_type = COP2K::Operand::REGADDR;
        $$.src = $2;
        $$.dst = 3;
    }
    | '#' expression ',' '#' expression {
        $$.src_type = COP2K::Operand::IMMED;
        $$.dst_type = COP2K::Operand::IMMED;
        $$.src = $2;
        $$.dst = $5;
    }
    | '#' expression ',' expression {
        $$.src_type = COP2K::Operand::IMMED;
        $$.dst_type = COP2K::Operand::MEMADDR;
        $$.src = $2;
        $$.dst = $4;
    }
    | expression ',' 'a' {
        $$.src_type = COP2K::Operand::MEMADDR;
        $$.dst_type = COP2K::Operand::REG_A;
        $$.src = $1;
    }
    | expression ',' R0 {
        $$.src_type = COP2K::Operand::MEMADDR;
        $$.dst_type = COP2K::Operand::REG;
        $$.src = $1;
        $$.dst = 0;
    }
    | expression ',' R1 {
        $$.src_type = COP2K::Operand::MEMADDR;
        $$.dst_type = COP2K::Operand::REG;
        $$.src = $1;
        $$.dst = 1;
    }
    | expression ',' R2 {
        $$.src_type = COP2K::Operand::MEMADDR;
        $$.dst_type = COP2K::Operand::REG;
        $$.src = $1;
        $$.dst = 2;
    }
    | expression ',' R3 {
        $$.src_type = COP2K::Operand::MEMADDR;
        $$.dst_type = COP2K::Operand::REG;
        $$.src = $1;
        $$.dst = 3;
    }
    | expression ',' AT_R0 {
        $$.src_type = COP2K::Operand::MEMADDR;
        $$.dst_type = COP2K::Operand::REGADDR;
        $$.src = $1;
        $$.dst = 0;
    }
    | expression ',' AT_R1 {
        $$.src_type = COP2K::Operand::MEMADDR;
        $$.dst_type = COP2K::Operand::REGADDR;
        $$.src = $1;
        $$.dst = 1;
    }
    | expression ',' AT_R2 {
        $$.src_type = COP2K::Operand::MEMADDR;
        $$.dst_type = COP2K::Operand::REGADDR;
        $$.src = $1;
        $$.dst = 2;
    }
    | expression ',' AT_R3 {
        $$.src_type = COP2K::Operand::MEMADDR;
        $$.dst_type = COP2K::Operand::REGADDR;
        $$.src = $1;
        $$.dst = 3;
    }
    | expression ',' '#' expression {
        $$.src_type = COP2K::Operand::MEMADDR;
        $$.dst_type = COP2K::Operand::IMMED;
        $$.src = $1;
        $$.dst = $4;
    }
    | expression ',' expression {
        $$.src_type = COP2K::Operand::MEMADDR;
        $$.dst_type = COP2K::Operand::MEMADDR;
        $$.src = $1;
        $$.dst = $3;
    }
;

expression
    : NUMBER
    | IDENTIFIER {
        try {
            $$ = current_as->consts.at($1);
        } catch (const std::out_of_range &) {
            free($1);
            yyerror("constant not found");
            YYERROR;
        }

        free($1);
    }
    | expression '+' expression      { $$ = $1 + $3; }
    | expression '-' expression      { $$ = $1 - $3; }
    | expression '*' expression      { $$ = $1 * $3; }
    | expression '/' expression      { $$ = $1 / $3; }
    | expression '%' expression      { $$ = $1 % $3; }
    | expression '<' expression      { $$ = $1 << $3; }
    | expression '>' expression      { $$ = $1 >> $3; }
    | expression '^' expression      { $$ = $1 ^ $3; }
    | expression '&' expression      { $$ = $1 & $3; }
    | expression '|' expression      { $$ = $1 | $3; }
    | '!' expression                 { $$ = ~$2; }
    | '+' expression   %prec UNPOS   { $$ = +$2; }
    | '-' expression   %prec UNNEG   { $$ = -$2; }
    | '(' expression ')'             { $$ = $2; }
;

%%

void COP2K::assemble(FILE *in, AS *as)
{
    yyasmin = in;
    current_as = as;

    int result = yyparse();

    current_as = nullptr;

    if (result)
        throw std::runtime_error("failed to assemble file");
}
