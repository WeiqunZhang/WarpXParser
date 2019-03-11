
%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include "wp_parser_y.h"
    int yylex (void);
%}

/*%define api.pure full */

%union {
    struct wp_node* n;
    double d;
    struct wp_symbol* s;
    enum wp_f1_t f1;
    enum wp_f2_t f2;
}

%token <n>  NODE
%token <d>  NUMBER
%token <s>  SYMBOL
%token <f1> F1
%token <f2> F2
%token EOL

%token POW "**"

%nonassoc F1 F2
%right '='
%left '+' '-'
%left '*' '/'
%left '<' '>'
%nonassoc NEG
%right POW

/* The type of expression are struct wp_node* */
%type <n> exp

%start input

%%

input:
  %empty
| input exp EOL {
    wp_defexpr($2);
  }
;

exp:
  NUMBER                     { $$ = wp_newnumber($1); }
| SYMBOL                     { $$ = wp_newsymbol($1); }
| exp '+' exp                { $$ = wp_newnode(WP_ADD, $1, $3); }
| exp '-' exp                { $$ = wp_newnode(WP_SUB, $1, $3); }
| exp '*' exp                { $$ = wp_newnode(WP_MUL, $1, $3); }
| exp '/' exp                { $$ = wp_newnode(WP_DIV, $1, $3); }
| '(' exp ')'                { $$ = $2; }
| exp '<' exp                { $$ = wp_newf2(WP_LT, $1, $3); }
| exp '>' exp                { $$ = wp_newf2(WP_GT, $1, $3); }
| '-'exp %prec NEG           { $$ = wp_newnode(WP_NEG, $2, NULL); }
| exp POW exp                { $$ = wp_newf2(WP_POW, $1, $3); }
| F1 '(' exp ')'             { $$ = wp_newf1($1, $3); }
| F2 '(' exp ',' exp ')'     { $$ = wp_newf2($1, $3, $5); }
;

%%
