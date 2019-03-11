#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wp_parser_c.h"
#include "wp_parser_y.h"
#include "wp_parser.lex.h"
#include "wp_parser.tab.h"

void*
wp_c_parser_new (char const* body)
{
    YY_BUFFER_STATE buffer = yy_scan_string(body);
    yyparse();
    void* parser = wp_parser_new();
    yy_delete_buffer(buffer);
    return parser;
}

void
wp_c_parser_delete (void* parser)
{
    wp_parser_delete(parser);
}

void*
wp_c_parser_dup (void* source)
{
    return wp_parser_dup(source);
}

void
wp_c_parser_regvar (void* parser, char const* name, double* p)
{
    wp_parser_regvar(parser, name, p);
}

void
wp_c_parser_setconst (void* parser, char const* name, double c)
{
    wp_parser_setconst(parser, name, c);
}
