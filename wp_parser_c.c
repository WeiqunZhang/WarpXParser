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
    void* parser = wp_parser_new();
    wp_parser_optimize(parser);
    yy_delete_buffer(buffer);
    return parser;
}

void*
wp_c_parserdup (void* orig_parser)
{
    return wp_parser_dup(orig_parser);
}

double
wp_c_eval (void* parser, double const* dp)
{
    return wp_parser_eval(parser, dp);
}

void
wp_c_finalize (void* parser)
{
    wp_parser_finalize(parser);
}
