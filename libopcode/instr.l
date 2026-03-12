%{
#include <iostream>
#include <cstring>

#include "libopcode.hpp"
#include "libopcode_yacc.hpp"
#include "instr.parser.hpp"
%}

%option caseless noyywrap yylineno

%s OPERAND_STATE

%%

<OPERAND_STATE>"A"     { return OPERAND_REG_A; }
<OPERAND_STATE>"MM"    { return OPERAND_MEMADDR; }
<OPERAND_STATE>"R?"    { return OPERAND_REG; }
<OPERAND_STATE>"#II"   { return OPERAND_IMMED; }
<OPERAND_STATE>"@R?"   { return OPERAND_REGADDR; }
"jump-on-zero"         { return JUMP_ON_ZERO_MARKER; }
"jump-on-carry"        { return JUMP_ON_CARRY_MARKER; }
"jump-unconditional"   { return JUMP_UNCONDITIONAL_MARKER; }

[A-Z_][A-Z0-9._]* {
    yyinstrlval.identifier_v = strdup(yyinstrtext);
    BEGIN(OPERAND_STATE);
    return IDENTIFIER;
}

"@"                             { BEGIN(INITIAL); return '@'; }
[!,:;]                          { return yyinstrtext[0]; }
[1-9][0-9]*|0x[0-9a-f]+|0[0-7]* { yyinstrlval.number_v = strtol(yyinstrtext, nullptr, 0); return NUMBER; }
"//"[^\n]*                      |
[\t\n\r\v ]+                    ; // ignore

. {
    std::cerr << "error: invalid character '"
        << yyinstrtext << "' at line "
        << yyinstrlineno << "." << std::endl;
    yyterminate();
}

%%
