%{
#include <iostream>

#include "as.hpp"
#include "instr.parser.hpp"
%}

%option caseless noyywrap yylineno

%s OPERAND_STATE

%%

<OPERAND_STATE>"A"   { return OPERAND_REG_A; }
<OPERAND_STATE>"MM"  { return OPERAND_MEMADDR; }
<OPERAND_STATE>"R?"  { return OPERAND_REG; }
<OPERAND_STATE>"#II" { return OPERAND_IMMED; }
<OPERAND_STATE>"@R?" { return OPERAND_REGADDR; }
"jump-on-zero"       { return JUMP_ON_ZERO_MARKER; }
"jump-on-carry"      { return JUMP_ON_CARRY_MARKER; }

[A-Z_][A-Z0-9._]* {
    yylval.identifier_v = strdup(yytext);
    BEGIN(OPERAND_STATE);
    return IDENTIFIER;
}

";"                             { BEGIN(INITIAL); return ';'; }
[!,:@]                          { return yytext[0]; }
[1-9][0-9]*|0x[0-9a-f]+|0[0-7]* { yylval.number_v = strtol(yytext, nullptr, 0); return NUMBER; }
"#"[^\n]*                       |
[\t\n\r\v ]+                    ; // ignore

. {
    std::cerr << "error: invalid character '"
        << yytext << "' at line "
        << yylineno << "." << std::endl;
    yyterminate();
}

%%
