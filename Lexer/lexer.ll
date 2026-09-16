%{
    #include <string>
    #include "../Interpreter/Interpreter.hpp"
    #include "parser.hpp"
    #define YY_DECL int yylex()
%}

%option noyywrap
%option yylineno

%%

[ \t]+ {}
\n {return NEWLINE;}

"<." {return INT_ARR_START;}
"<," {return BOOL_ARR_START;}
"<$" {return PROC_ARR_START;}
">" {return ARR_END;}
":" {return COLON;}
"<-" {return ASSIGN;}
"-" {return DASH;}
".#" {return PIERCE_ARROW;}

"~"[0-9]+ {
    yylval.int_val = std::atoi(yytext + 1);
    return LABEL_LIT; 
}

",#"[0-9]+ {
    yylval.int_val = std::atoi(yytext + 2);
    return OP_INC;
}
",*"[0-9]+ {
    yylval.int_val = std::atoi(yytext + 2);
    return OP_DEC;
}

","[0-9]+ {
    yylval.int_val = std::atoi(yytext + 1);
    return INT_VAR;
}
"."[0-9]+ {
    yylval.int_val = std::atoi(yytext + 1);
    return BOOL_VAR;
}
"$"[0-9]+ {
    yylval.int_val = std::atoi(yytext + 1);
    return PROC_VAR;
}
[0-9]+ {
    yylval.int_val = std::atoi(yytext);
    return INT_LITERAL;
}

"T" {
    yylval.bool_val = true;
    return BOOL_TRUE;
}
"F" {
    yylval.bool_val = false;
    return BOOL_FALSE;
}

"eq" {return OP_EQ;}
"mf" {return CMD_MF;}
"mb" {return CMD_MB;}
"mr" {return CMD_MR;}
"ml" {return CMD_ML;}
"tp" {return CMD_TP;}
"np" {return CMD_NP;}
"please" {return PLEASE;}
"@" {return OP_BIND;}
"%" {return OP_UNBIND;}
"!" {return OP_NOT;}
"[[" {return L_COND;}
"]]" {return R_COND;}
"{" {return L_BRACE;}
"}" {return R_BRACE;}
"(" {return L_PAREN;}
")" {return R_PAREN;}

. {return ERROR_TOKEN;}

%%