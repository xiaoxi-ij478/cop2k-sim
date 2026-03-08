%{
#include <iostream>

#include "as.hpp"
#include "instr.parser.hpp"
%}

%option caseless noyywrap yylineno

%%

"A"                                { return OPERAND_REG_A; }
"MM"                               { return OPERAND_MEMADDR; }
"R?"                               { return OPERAND_REG; }
"#II"                              { return OPERAND_IMMED; }
"@R?"                              { return OPERAND_REGADDR; }

[A-Z_][A-Z0-9._]*                  { yylval.identifier_v = strdup(yytext); return IDENTIFIER; }

[!,@:;]                            { return yytext[0]; }
0|[1-9][0-9]*|0x[0-9a-f]+|0[0-7]+  { yylval.number_v = strtol(yytext, nullptr, 0); return NUMBER; }

[\t\n\r\v ]+                       ; // ignore

. {
    std::cerr << "error: invalid character '"
        << yytext << "' at line "
        << yylineno << "." << std::endl;
}

%%
