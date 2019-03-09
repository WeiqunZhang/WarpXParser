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
"(" |
")"     { return yytext[0]; }

 /* keywords */
"lambda"   { return LAMBDA; }
"eval"     { return EVAL; }

 /* functions */
"sqrt"     { yylval.f1 = WP_SQRT;   return FNCT1; }
"exp"      { yylval.f1 = WP_EXP;    return FNCT1; }
"log"      { yylval.f1 = WP_LOG;    return FNCT1; }
"log10"    { yylval.f1 = WP_LOG10;  return FNCT1; }
"sin"      { yylval.f1 = WP_SIN;    return FNCT1; }
"cos"      { yylval.f1 = WP_COS;    return FNCT1; }
"tan"      { yylval.f1 = WP_TAN;    return FNCT1; }
"asin"     { yylval.f1 = WP_ASIN;   return FNCT1; }
"acos"     { yylval.f1 = WP_ACOS;   return FNCT1; }
"atan"     { yylval.f1 = WP_ATAN;   return FNCT1; }
"sinh"     { yylval.f1 = WP_SINH;   return FNCT1; }
"cosh"     { yylval.f1 = WP_COSH;   return FNCT1; }
"tanh"     { yylval.f1 = WP_TANH;   return FNCT1; }

"pow"      { yylval.f2 = WP_POW;    return FNCT2; }

 /* symbols */
[a-zA-Z][a-zA-Z0-9]*  { yylval.s = wp_lookup(yytext); return SYMBOL; }

 /* number */
[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}?  { yylval.d = atof(yytext);   return NUM; }

 /* special characters */
"//".*  
[ \t]   /* ignore white space */ 
\\\n    /* ignore line continuation */
"\n"    { return EOL; }

 /* everything else */
.	{ yyerror("Unknow character %c\n", *yytext); }

%%
