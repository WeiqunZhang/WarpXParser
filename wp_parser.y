
%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include "wp_parser_y.h"
    int yylex (void);
%}

/*%define api.pure full */

/* %parse-parm */

%union {
    struct wp_node* a;
    struct wp_symbol* s;
    struct wp_symlist* l;
    enum wp_f1_t f1;
    enum wp_f2_t f2;
    double d;
}

%token <d>  NUM
%token <s>  SYMBOL
%token <f1> FNCT1
%token <f2> FNCT2
%token EOL

%token POW "**"

%token LAMBDA EVAL

%nonassoc FNCT1 FNCT2
%right '='
%left '+' '-'
%left '*' '/'
%left '<' '>'
%nonassoc NEG
%right POW

%type <a> exp explist
%type <l> symlist

%start input

%%

input:
  %empty
| input EVAL '(' explist ')' EOL {
    printf("= %g\n> ", wp_evallambda($4));
  }
| input LAMBDA '(' symlist ')' '=' exp EOL {
    wp_deflambda($4, $7);
  }
| input error EOL { yyerrok; printf("> "); }
;

exp:
  NUM                        { $$ = wp_newnum($1); }
| SYMBOL                     { $$ = wp_newsymref($1); }
| exp '+' exp                { $$ = wp_newnode(WP_ADD, $1, $3); }
| exp '-' exp                { $$ = wp_newnode(WP_SUB, $1, $3); }
| exp '*' exp                { $$ = wp_newnode(WP_MUL, $1, $3); }
| exp '/' exp                { $$ = wp_newnode(WP_DIV, $1, $3); }
| '(' exp ')'                { $$ = $2; }
| exp '<' exp                { $$ = wp_newf2(WP_LT, $1, $3); }
| exp '>' exp                { $$ = wp_newf2(WP_GT, $1, $3); }
| '-'exp %prec NEG           { $$ = wp_newnode(WP_NEG, $2, NULL); }
| exp POW exp                { $$ = wp_newf2(WP_POW, $1, $3); }
| FNCT1 '(' exp ')'          { $$ = wp_newf1($1, $3); }
| FNCT2 '(' exp ',' exp ')'  { $$ = wp_newf2($1, $3, $5); }
;

explist:
  exp
| exp ',' explist      { $$ = wp_newnode(WP_LIST, $1, $3); }
;

symlist:
  SYMBOL             { $$ = wp_newsymlist($1, NULL); }
| SYMBOL ',' symlist { $$ = wp_newsymlist($1, $3); }
;

%%
