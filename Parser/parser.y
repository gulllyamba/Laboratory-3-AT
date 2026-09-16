%{
    #include <iostream>
    #include <string>
    #include <map>
    #include <vector>
    #include <memory>
    #include "../Interpreter/Interpreter.hpp"

    extern int yylex();
    extern int yylineno;
    void yyerror(const char *s);

    std::string parser_error_message = "";
    std::shared_ptr<ASTNode> root_node = nullptr;
%}

%union {
    int int_val;
    bool bool_val;
    ASTNode* node_ptr;
    ASTBlock* block_ptr;
    ASTExpression<int>* int_expr_ptr;
    ASTExpression<bool>* bool_expr_ptr;
    std::vector<ASTExpression<int>*>* index_list_ptr;
}

%token <int_val> INT_LITERAL INT_VAR BOOL_VAR PROC_VAR LABEL_LIT OP_INC OP_DEC
%token <bool_val> BOOL_TRUE BOOL_FALSE
%token CMD_MF CMD_MB CMD_MR CMD_ML CMD_TP CMD_NP
%token ASSIGN PIERCE_ARROW OP_EQ PLEASE OP_BIND OP_UNBIND OP_NOT
%token L_COND R_COND L_BRACE R_BRACE L_PAREN R_PAREN COLON DASH NEWLINE ERROR_TOKEN
%token INT_ARR_START BOOL_ARR_START PROC_ARR_START ARR_END

%type <node_ptr> line statement no_command
%type <block_ptr> program block_statements
%type <int_expr_ptr> int_expression
%type <bool_expr_ptr> bool_expression
%type <index_list_ptr> index_list

%precedence OP_NOT
%left PIERCE_ARROW
%left OP_EQ

%start start

%%

start:
    program {
        root_node = std::shared_ptr<ASTNode>($1);
    }
;

program:
    /* пусто */ {
        $$ = new ASTBlock();
    }
    | program line {
        if ($2 != nullptr) {
            $1->add_statement($2);
        }
        $$ = $1;
    }
;

line:
    NEWLINE {$$ = nullptr;}
    | statement NEWLINE {$$ = $1;}
    | error NEWLINE {$$ = nullptr;}
;

statement:
    bool_expression {$$ = new ASTExpressionStatement($1);}
    | no_command {$$ = $1;}
    | block_statements {$$ = $1;}
    | INT_VAR ASSIGN int_expression {$$ = new ASTIntAssign($1, $3);}
    | BOOL_VAR ASSIGN bool_expression {$$ = new ASTBoolAssign($1, $3);}
    | OP_INC {$$ = new ASTIntIncrement($1);}
    | OP_DEC {$$ = new ASTIntDecrement($1);}
    | PROC_VAR ASSIGN PROC_ARR_START INT_LITERAL COLON index_list ARR_END {
        $$ = new ASTProcAssign($1, $4, *$6);
        delete $6;
    }
    | PROC_VAR ASSIGN block_statements {$$ = new ASTProcDefine($1, $3);}
    | PROC_VAR ASSIGN PROC_VAR {$$ = new ASTProcCopy($1, $3);}
    | PROC_VAR ASSIGN CMD_NP {$$ = new ASTProcDefine($1, new ASTNoCommand());}
    | PROC_VAR {$$ = new ASTProcCall($1);}
    | L_PAREN bool_expression R_PAREN statement {$$ = new ASTWhileLoop($2, $4);}
    | LABEL_LIT {$$ = new ASTLabel($1);}
    | L_COND bool_expression R_COND LABEL_LIT {$$ = new ASTGotoStatement($2, $4, false);}
    | L_COND bool_expression R_COND PLEASE LABEL_LIT {$$ = new ASTGotoStatement($2, $5, true);}
    | INT_ARR_START INT_LITERAL COLON index_list ARR_END ASSIGN int_expression {
        $$ = new ASTIntArrayAssign($2, *$4, $7);
        delete $4;
    }
    | BOOL_ARR_START INT_LITERAL COLON index_list ARR_END ASSIGN bool_expression {
        $$ = new ASTBoolArrayAssign($2, *$4, $7);
        delete $4;
    }
    | PROC_ARR_START INT_LITERAL COLON index_list ARR_END ASSIGN block_statements {
        $$ = new ASTProcArrayDefine($2, *$4, $7);
        delete $4;
    }
    | PROC_ARR_START INT_LITERAL COLON index_list ARR_END ASSIGN PROC_VAR {
        $$ = new ASTProcArrayAssign($2, *$4, $7);
        delete $4;
    }
    | PROC_ARR_START INT_LITERAL COLON index_list ARR_END ASSIGN CMD_NP {
        $$ = new ASTProcArrayDefine($2, *$4, new ASTNoCommand());
        delete $4;
    }
    | PROC_ARR_START INT_LITERAL COLON index_list ARR_END {
        $$ = new ASTProcArrayCall($2, *$4);
        delete $4;
    }
;

int_expression:
    INT_LITERAL {$$ = new ASTIntLiteral($1);}
    | DASH INT_LITERAL {$$ = new ASTIntLiteral(-1 * $2);}
    | INT_VAR {$$ = new ASTIntVar($1);}
    | INT_ARR_START INT_LITERAL COLON index_list ARR_END {
        $$ = new ASTIntArrayAccess($2, *$4);
        delete $4;
    }
;

bool_expression:
    BOOL_TRUE {$$ = new ASTBoolLiteral(true);}
    | BOOL_FALSE {$$ = new ASTBoolLiteral(false);}
    | BOOL_VAR {$$ = new ASTBoolVar($1);}
    | OP_NOT bool_expression {$$ = new ASTNotExpression($2);}
    | L_PAREN bool_expression R_PAREN {$$ = $2;}
    | bool_expression PIERCE_ARROW bool_expression {$$ = new ASTPierceArrow($1, $3);}
    | int_expression OP_EQ int_expression {$$ = new ASTIntEqExpression($1, $3);}
    | bool_expression OP_EQ bool_expression {$$ = new ASTBoolEqExpression($1, $3);}
    | PROC_VAR OP_EQ CMD_NP {$$ = new ASTProcEqNpExpression($1);}
    | CMD_MF {$$ = new ASTRobotCommand("mf");}
    | CMD_MR {$$ = new ASTRobotCommand("mr");}
    | CMD_MB {$$ = new ASTRobotCommand("mb");}
    | CMD_ML {$$ = new ASTRobotCommand("ml");}
    | CMD_TP {$$ = new ASTTeleportCommand();}
    | BOOL_ARR_START INT_LITERAL COLON index_list ARR_END {
        $$ = new ASTBoolArrayAccess($2, *$4);
        delete $4;
    }
    | INT_VAR OP_BIND PROC_VAR {$$ = new ASTBindOperator("int", $1, $3);}
    | BOOL_VAR OP_BIND PROC_VAR {$$ = new ASTBindOperator("bool", $1, $3);}
    | PROC_VAR OP_BIND PROC_VAR {$$ = new ASTBindOperator("proc", $1, $3);}
    | INT_VAR OP_UNBIND PROC_VAR {$$ = new ASTUnbindOperator("int", $1, $3);}
    | BOOL_VAR OP_UNBIND PROC_VAR {$$ = new ASTUnbindOperator("bool", $1, $3);}
    | PROC_VAR OP_UNBIND PROC_VAR {$$ = new ASTUnbindOperator("proc", $1, $3);}
;

no_command:
    CMD_NP {$$ = new ASTNoCommand();}
;

block_statements:
    L_BRACE program R_BRACE {$$ = $2;}
;

index_list:
    int_expression {
        $$ = new std::vector<ASTExpression<int>*>();
        $$->push_back($1);
    }
    | index_list DASH int_expression {
        $$ = $1;
        $$->push_back($3);
    }
;

%%

void yyerror(const char *s) {
    parser_error_message = "Ошибка в строке " + std::to_string(yylineno) + ": " + s;
    std::cerr << parser_error_message << std::endl;
}