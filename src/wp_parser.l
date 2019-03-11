%option noyywrap nodefault yylineno
%{
#include "wp_parser_y.h"
#include "wp_parser.tab.h"
%}

EXP	([Ee][-+]?[0-9]+)

%%
 /* operators */
"+" |
"-" |
"*" |
"/" |
"=" |
"|" |
"," |
"<" |
">" |
"(" |
")"     { return yytext[0]; }

"sqrt"     { yylval.f1 = WP_SQRT;   return F1; }
"exp"      { yylval.f1 = WP_EXP;    return F1; }
"log"      { yylval.f1 = WP_LOG;    return F1; }
"log10"    { yylval.f1 = WP_LOG10;  return F1; }
"sin"      { yylval.f1 = WP_SIN;    return F1; }
"cos"      { yylval.f1 = WP_COS;    return F1; }
"tan"      { yylval.f1 = WP_TAN;    return F1; }
"asin"     { yylval.f1 = WP_ASIN;   return F1; }
"acos"     { yylval.f1 = WP_ACOS;   return F1; }
"atan"     { yylval.f1 = WP_ATAN;   return F1; }
"sinh"     { yylval.f1 = WP_SINH;   return F1; }
"cosh"     { yylval.f1 = WP_COSH;   return F1; }
"tanh"     { yylval.f1 = WP_TANH;   return F1; }
"abs"      { yylval.f1 = WP_ABS;    return F1; }
"**"       { yylval.f2 = WP_POW;    return POW; }
"pow"      { yylval.f2 = WP_POW;    return F2; }

[a-zA-Z][a-zA-Z0-9]*  { yylval.s = wp_makesymbol(yytext); return SYMBOL; }

 /* number */
[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}?  { yylval.d = atof(yytext);   return NUMBER; }

 /* special characters */
"//".*  
[ \t]   /* ignore white space */ 
\\\n    /* ignore line continuation */
"\n"    { return EOL; }

 /* everything else */
.	{ yyerror("Unknow character %c\n", *yytext); }

%%
