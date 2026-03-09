%{
#include <iostream>
#include <cstring>

#include "as.hpp"
#include "asm.parser.hpp"
%}

%option caseless noyywrap yylineno

%s OPERAND_STATE
%%

<OPERAND_STATE>"a"    { return 'a'; }
<OPERAND_STATE>"equ"  { return EQU; }
<OPERAND_STATE>"r0"   { return R0; }
<OPERAND_STATE>"r1"   { return R1; }
<OPERAND_STATE>"r2"   { return R2; }
<OPERAND_STATE>"r3"   { return R3; }
<OPERAND_STATE>"@r0"  { return AT_R0; }
<OPERAND_STATE>"@r1"  { return AT_R1; }
<OPERAND_STATE>"@r2"  { return AT_R2; }
<OPERAND_STATE>"@r3"  { return AT_R3; }
"db"                  { return DB; } // these instructions are restricted at instruction parse stage
"org"                 { return ORG; }
"end"                 { return END; }
"if"                  { return IF; }
"else"                { return ELSE; }
"endif"               { return ENDIF; }

[A-Z_][A-Z0-9_.]* {
    yylval.identifier_v = strdup(yytext);
    BEGIN(OPERAND_STATE);
    return IDENTIFIER;
}

"#" | // comment
":" | // label definer
"," | // operand separator
[!*/+^&|<>%()-] { // operators
    /* why there's no '@'?
     * because @ is only used in register memory addressing
     * and it may not appear anywhere else
     */
    return yytext[0];
}

([1-9][0-9]*|0x[0-9a-f]+|0[0-7]*|0b[01]+)   {
    yylval.number_v = strtol(yytext, nullptr, 0);
    return NUMBER;
}
[0-7]+o {
    yylval.number_v = strtol(yytext, nullptr, 8);
    return NUMBER;
}
[0-9a-f]+h {
    yylval.number_v = strtol(yytext, nullptr, 16);
    return NUMBER;
}
[01]+b {
    yylval.number_v = strtol(yytext, nullptr, 2);
    return NUMBER;
}

[\t\r\v ]+ |
";"[^\n]*  ; // ignore
\n         { BEGIN(INITIAL); return '\n'; }

. {
    std::cerr << "error: invalid character '"
        << yytext << "' at line "
        << yylineno << "." << std::endl;
    yyterminate();
}

%%
