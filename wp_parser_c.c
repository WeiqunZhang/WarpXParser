#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wp_parser_c.h"
#include "wp_parser_y.h"
#include "wp_parser.lex.h"
#include "wp_parser.tab.h"

void*
wp_c_parse (char const* body)
{
    YY_BUFFER_STATE buffer = yy_scan_string(body);
    yyparse();
//    double a[2] = {2.2,2.8};
//    printf("eval(2.2,2.8) = %g\n", wp_eval_dp(a));
//    yy_delete_buffer(buffer);
    return NULL;
}

double
wp_c_eval (double const* dp)
{
    return wp_eval_dp(dp);
}
