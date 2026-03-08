%{
#include "as.hpp"
#include "asm.parser.hpp"

#include <iostream>
%}

%option caseless noyywrap yylineno

%%

db   {
    return DATA_BYTE;
}

[A-Z_][A-Z0-9_.]*   {
    yylval.identifier_v = strdup(yytext);
    return IDENTIFIER;
}
[#,;:@]   {
    return yytext[0];
}
[-+]?(0|[1-9][0-9]*|0x[0-9a-f]+|0[0-7]+|[0-9a-f]+h)   {
    yylval.number_v = strtol(yytext, nullptr, 0);
    return NUMBER;
}
