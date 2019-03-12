#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "wp_parser_y.h"
#include "wp_parser.tab.h"

static struct wp_node* wp_root = NULL;

/* This is called by a bison rule to store the original AST in a
 * static variable.  Accessing this directly is not thread safe.  So
 * this will be duplicated later for each thread.
 */
void
wp_defexpr (struct wp_node* body)
{
    wp_root = body;
}

struct wp_node*
wp_newnumber (double d)
{
    struct wp_number* r = malloc(sizeof(struct wp_number));
    r->type = WP_NUMBER;
    r->value = d;
    return (struct wp_node*) r;
}

struct wp_symbol*
wp_makesymbol (char* name)
{
    struct wp_symbol* symbol = malloc(sizeof(struct wp_symbol));
    symbol->type = WP_SYMBOL;
    symbol->name = strdup(name);
    symbol->pointer = NULL;
    return symbol;
}

struct wp_node*
wp_newsymbol (struct wp_symbol* symbol)
{
    return (struct wp_node*) symbol;
}

struct wp_node*
wp_newnode (enum wp_node_t type, struct wp_node* l, struct wp_node* r)
{
    struct wp_node* tmp = malloc(sizeof(struct wp_node));
    tmp->type = type;
    tmp->l = l;
    tmp->r = r;
    return tmp;
}

struct wp_node*
wp_newf1 (enum wp_f1_t ftype, struct wp_node* l)
{
    struct wp_f1* tmp = malloc(sizeof(struct wp_f1));
    tmp->type = WP_F1;
    tmp->l = l;
    tmp->ftype = ftype;
    return (struct wp_node*) tmp;
}

struct wp_node*
wp_newf2 (enum wp_f2_t ftype, struct wp_node* l, struct wp_node* r)
{
    struct wp_f2* tmp = malloc(sizeof(struct wp_f2));
    tmp->type = WP_F2;
    tmp->l = l;
    tmp->r = r;
    tmp->ftype = ftype;
    return (struct wp_node*) tmp;
}

void
yyerror (char const *s, ...)
{
    va_list vl;
    va_start(vl, s);
    vfprintf(stderr, s, vl);
    fprintf(stderr, "\n");
    va_end(vl);
}

/*******************************************************************/

struct wp_parser*
wp_parser_new (void)
{
    struct wp_parser* my_parser = malloc(sizeof(struct wp_parser));

    my_parser->sz_mempool = wp_ast_size(wp_root);
    my_parser->p_root = malloc(my_parser->sz_mempool);
    my_parser->p_free = my_parser->p_root;

    my_parser->ast = wp_parser_ast_dup(my_parser, wp_root,1); /* 1: free the source wp_root */

    if (my_parser->p_root + my_parser->sz_mempool != my_parser->p_free) {
        yyerror("wp_parser_new: error in memory size");
        exit(1);
    }

    wp_ast_optimize(my_parser->ast);

    return my_parser;
}

void
wp_parser_delete (struct wp_parser* parser)
{
    free(parser->p_root);
    free(parser);
}

static size_t
wp_aligned_size (size_t N)
{
    const unsigned int align_size = 16;
    size_t x = N + (align_size-1);
    x -= x & (align_size-1);
    return x;
}

static
void*
wp_parser_allocate (struct wp_parser* my_parser, size_t N)
{
    void* r = my_parser->p_free;
    my_parser->p_free = (char*)r + wp_aligned_size(N);
    return r;
}

struct wp_parser*
wp_parser_dup (struct wp_parser* source)
{
    struct wp_parser* dest = malloc(sizeof(struct wp_parser));
    dest->sz_mempool = source->sz_mempool;
    dest->p_root = malloc(dest->sz_mempool);
    dest->p_free = dest->p_root;

    dest->ast = wp_parser_ast_dup(dest, source->ast, 0); /* 0: don't free the source */

    return dest;
}

static
double
wp_callf1 (struct wp_f1* f1)
{
    double a = wp_ast_eval(f1->l);
    switch (f1->ftype) {
    case WP_SQRT:        return sqrt(a);
    case WP_EXP:         return exp(a);
    case WP_LOG:         return log(a);
    case WP_LOG10:       return log10(a);
    case WP_SIN:         return sin(a);
    case WP_COS:         return cos(a);
    case WP_TAN:         return tan(a);
    case WP_ASIN:        return asin(a);
    case WP_ACOS:        return acos(a);
    case WP_ATAN:        return atan(a);
    case WP_SINH:        return sinh(a);
    case WP_COSH:        return cosh(a);
    case WP_TANH:        return tanh(a);
    case WP_ABS:         return fabs(a);
    default:
        yyerror("wp_callf1: Unknow function %d", f1->ftype);
        return 0.0;
    }
}

static
double
wp_callf2 (struct wp_f2* f2)
{
    double a = wp_ast_eval(f2->l);
    double b = wp_ast_eval(f2->r);
    switch (f2->ftype) {
    case WP_POW:
        return pow(a,b);
    case WP_GT:
        return (a > b) ? 1.0 : 0.0;
    case WP_LT:
        return (a < b) ? 1.0 : 0.0;
    case WP_HEAVISIDE:
        return (a < 0.0) ? 0.0 : ((a > 0.0) ? 1.0 : b);
    default:
        yyerror("wp_callf2: Unknow function %d", f2->ftype);
        return 0.0;
    }
}

double
wp_ast_eval (struct wp_node* node)
{
    double result;

    switch (node->type)
    {
    case WP_NUMBER:
        result = ((struct wp_number*)node)->value;
        break;
    case WP_SYMBOL:
        result = *(((struct wp_symbol*)node)->pointer);
        break;
    case WP_ADD:
        result = wp_ast_eval(node->l) + wp_ast_eval(node->r);
        break;
    case WP_SUB:
        result = wp_ast_eval(node->l) - wp_ast_eval(node->r);
        break;
    case WP_MUL:
        result = wp_ast_eval(node->l) * wp_ast_eval(node->r);
        break;
    case WP_DIV:
        result = wp_ast_eval(node->l) / wp_ast_eval(node->r);
        break;
    case WP_NEG:
        result = -wp_ast_eval(node->l);
        break;
    case WP_F1:
        result = wp_callf1((struct wp_f1*)node);
        break;
    case WP_F2:
        result = wp_callf2((struct wp_f2*)node);
        break;
    case WP_ADDR:
        result = ((struct wp_node_r*)node)->value + wp_ast_eval(node->r);
        break;
    case WP_SUBR:
        result = ((struct wp_node_r*)node)->value - wp_ast_eval(node->r);
        break;
    case WP_MULR:
        result = ((struct wp_node_r*)node)->value * wp_ast_eval(node->r);
        break;
    case WP_DIVR:
        result = ((struct wp_node_r*)node)->value / wp_ast_eval(node->r);
        break;
    default:
        yyerror("wp_ast_eval: unknown node type %d\n", node->type);
    }

    return result;
}

size_t
wp_ast_size (struct wp_node* node)
{
    size_t result;

    switch (node->type)
    {
    case WP_NUMBER:
        result = wp_aligned_size(sizeof(struct wp_number));
        break;
    case WP_SYMBOL:
        result = wp_aligned_size(sizeof(struct wp_symbol))
            + wp_aligned_size(strlen(((struct wp_symbol*)node)->name)+1);
        break;
    case WP_ADD:
    case WP_SUB:
    case WP_MUL:
    case WP_DIV:
        result = wp_aligned_size(sizeof(struct wp_node))
            + wp_ast_size(node->l) + wp_ast_size(node->r);
        break;
    case WP_NEG:
        result = wp_aligned_size(sizeof(struct wp_node))
            + wp_ast_size(node->l);
        break;
    case WP_F1:
        result = wp_aligned_size(sizeof(struct wp_f1))
            + wp_ast_size(node->l);
        break;
    case WP_F2:
        result = wp_aligned_size(sizeof(struct wp_f2))
            + wp_ast_size(node->l) + wp_ast_size(node->r);
        break;
    case WP_ADDR:
    case WP_SUBR:
    case WP_MULR:
    case WP_DIVR:
        result = wp_aligned_size(sizeof(struct wp_node_r))
            + wp_ast_size(node->r);
        break;
    default:
        yyerror("wp_ast_size: unknown node type %d\n", node->type);
        exit(1);
    }

    return result;
}

struct wp_node*
wp_parser_ast_dup (struct wp_parser* my_parser, struct wp_node* node, int move)
{
    void* result;

    switch (node->type)
    {
    case WP_NUMBER:
        result = wp_parser_allocate(my_parser, sizeof(struct wp_number));
        memcpy(result, node                  , sizeof(struct wp_number));
        break;
    case WP_SYMBOL:
        result = wp_parser_allocate(my_parser, sizeof(struct wp_symbol));
        memcpy(result, node                  , sizeof(struct wp_symbol));
        ((struct wp_symbol*)result)->name = wp_parser_allocate
            (my_parser, strlen(((struct wp_symbol*)node)->name)+1);
        strcpy(((struct wp_symbol*)result)->name,
               ((struct wp_symbol*)node  )->name);
        break;
    case WP_ADD:
    case WP_SUB:
    case WP_MUL:
    case WP_DIV:
        result = wp_parser_allocate(my_parser, sizeof(struct wp_node));
        memcpy(result, node                  , sizeof(struct wp_node));
        ((struct wp_node*)result)->l = wp_parser_ast_dup(my_parser, node->l, move);
        ((struct wp_node*)result)->r = wp_parser_ast_dup(my_parser, node->r, move);
        break;
    case WP_NEG:
        result = wp_parser_allocate(my_parser, sizeof(struct wp_node));
        memcpy(result, node                  , sizeof(struct wp_node));
        ((struct wp_node*)result)->l = wp_parser_ast_dup(my_parser, node->l, move);
        ((struct wp_node*)result)->r = NULL;
        break;
    case WP_F1:
        result = wp_parser_allocate(my_parser, sizeof(struct wp_f1));
        memcpy(result, node                  , sizeof(struct wp_f1));
        ((struct wp_f1*)result)->l = wp_parser_ast_dup(my_parser, ((struct wp_f1*)node)->l, move);
        break;
    case WP_F2:
        result = wp_parser_allocate(my_parser, sizeof(struct wp_f2));
        memcpy(result, node                  , sizeof(struct wp_f2));
        ((struct wp_f2*)result)->l = wp_parser_ast_dup(my_parser, ((struct wp_f2*)node)->l, move);
        ((struct wp_f2*)result)->r = wp_parser_ast_dup(my_parser, ((struct wp_f2*)node)->r, move);
        break;
    case WP_ADDR:
    case WP_SUBR:
    case WP_MULR:
    case WP_DIVR:
        result = wp_parser_allocate(my_parser, sizeof(struct wp_node_r));
        memcpy(result, node                  , sizeof(struct wp_node_r));
        ((struct wp_node_r*)result)->value = ((struct wp_node_r*)node)->value;
        ((struct wp_node_r*)result)->r = wp_parser_ast_dup(my_parser, node->r, move);
        break;
    default:
        yyerror("wp_ast_dup: unknown node type %d\n", node->type);
        exit(1);
    }
    if (move) {
        /* Note that we only do this on the original AST.  We do not
         * need to call free for AST stored in wp_parser because the
         * memory is not allocated with malloc directly.
         */
        if (node->type == WP_SYMBOL) {
            free(((struct wp_symbol*)node)->name);
        }
        free((void*)node);
    }
    return (struct wp_node*)result;
}

void
wp_ast_optimize (struct wp_node* node)
{
    /* No need to free memory because we only call this on ASTs in
     * wp_parser that are allocated from the memory pool.
     */
    switch (node->type)
    {
    case WP_NUMBER:
    case WP_SYMBOL:
        break;
    case WP_ADD:
        wp_ast_optimize(node->l);
        wp_ast_optimize(node->r);
        if (node->l->type == WP_NUMBER &&
            node->r->type == WP_NUMBER)
        {
            double v = wp_ast_eval(node);
            ((struct wp_number*)node)->type = WP_NUMBER;
            ((struct wp_number*)node)->value = v;
        }
        else if (node->l->type == WP_NUMBER)
        {
            double v = wp_ast_eval(node->l);
            ((struct wp_node_r*)node)->value = v;
            ((struct wp_node_r*)node)->type = WP_ADDR;
        }
        else if (node->r->type == WP_NUMBER)
        {
            struct wp_node* n = node->l;
            double v = wp_ast_eval(node->r);
            ((struct wp_node_r*)node)->r = n;
            ((struct wp_node_r*)node)->value = v;
            ((struct wp_node_r*)node)->type = WP_ADDR;
        }
        break;
    case WP_SUB:
        wp_ast_optimize(node->l);
        wp_ast_optimize(node->r);
        if (node->l->type == WP_NUMBER &&
            node->r->type == WP_NUMBER)
        {
            double v = wp_ast_eval(node);
            ((struct wp_number*)node)->type = WP_NUMBER;
            ((struct wp_number*)node)->value = v;
        }
        else if (node->l->type == WP_NUMBER)
        {
            double v = wp_ast_eval(node->l);
            ((struct wp_node_r*)node)->value = v;
            ((struct wp_node_r*)node)->type = WP_SUBR;
        }
        else if (node->r->type == WP_NUMBER)
        {
            struct wp_node* n = node->l;
            double v = wp_ast_eval(node->r);
            ((struct wp_node_r*)node)->r = n;
            ((struct wp_node_r*)node)->value = -v;
            ((struct wp_node_r*)node)->type = WP_ADDR;
        }
        break;
    case WP_MUL:
        wp_ast_optimize(node->l);
        wp_ast_optimize(node->r);
        if (node->l->type == WP_NUMBER &&
            node->r->type == WP_NUMBER)
        {
            double v = wp_ast_eval(node);
            ((struct wp_number*)node)->type = WP_NUMBER;
            ((struct wp_number*)node)->value = v;
        }
        else if (node->l->type == WP_NUMBER)
        {
            double v = wp_ast_eval(node->l);
            ((struct wp_node_r*)node)->value = v;
            ((struct wp_node_r*)node)->type = WP_MULR;
        }
        else if (node->r->type == WP_NUMBER)
        {
            struct wp_node* n = node->l;
            double v = wp_ast_eval(node->r);
            ((struct wp_node_r*)node)->r = n;
            ((struct wp_node_r*)node)->value = v;
            ((struct wp_node_r*)node)->type = WP_MULR;
        }
        break;
    case WP_DIV:
        wp_ast_optimize(node->l);
        wp_ast_optimize(node->r);
        if (node->l->type == WP_NUMBER &&
            node->r->type == WP_NUMBER)
        {
            double v = wp_ast_eval(node);
            ((struct wp_number*)node)->type = WP_NUMBER;
            ((struct wp_number*)node)->value = v;
        }
        else if (node->l->type == WP_NUMBER)
        {
            double v = wp_ast_eval(node->l);
            ((struct wp_node_r*)node)->value = v;
            ((struct wp_node_r*)node)->type = WP_DIVR;
        }
        else if (node->r->type == WP_NUMBER)
        {
            struct wp_node* n = node->l;
            double v = wp_ast_eval(node->r);
            ((struct wp_node_r*)node)->r = n;
            ((struct wp_node_r*)node)->value = 1.0/v;
            ((struct wp_node_r*)node)->type = WP_MULR;
        }
        break;
    case WP_NEG:
        wp_ast_optimize(node->l);
        if (node->l->type == WP_NUMBER)
        {
            double v = wp_ast_eval(node);
            ((struct wp_number*)node)->type = WP_NUMBER;
            ((struct wp_number*)node)->value = v;
        }
        break;
    case WP_F1:
        wp_ast_optimize(node->l);
        if (node->l->type == WP_NUMBER)
        {
            double v = wp_ast_eval(node);
            ((struct wp_number*)node)->type = WP_NUMBER;
            ((struct wp_number*)node)->value = v;
        }
        break;
    case WP_F2:
        wp_ast_optimize(node->l);
        wp_ast_optimize(node->r);
        if (node->l->type == WP_NUMBER &&
            node->r->type == WP_NUMBER)
        {
            double v = wp_ast_eval(node);
            ((struct wp_number*)node)->type = WP_NUMBER;
            ((struct wp_number*)node)->value = v;
        }
        break;
    case WP_ADDR:
        wp_ast_optimize(node->r);
        if (node->r->type == WP_NUMBER)
        {
            double v =        ((struct wp_node_r*)node)->value
                + wp_ast_eval(((struct wp_node_r*)node)->r);
            ((struct wp_number*)node)->type = WP_NUMBER;
            ((struct wp_number*)node)->value = v;
        }
        break;
    case WP_SUBR:
        wp_ast_optimize(node->r);
        if (node->r->type == WP_NUMBER)
        {
            double v =        ((struct wp_node_r*)node)->value
                - wp_ast_eval(((struct wp_node_r*)node)->r);
            ((struct wp_number*)node)->type = WP_NUMBER;
            ((struct wp_number*)node)->value = v;
        }
        break;
    case WP_MULR:
        wp_ast_optimize(node->r);
        if (node->r->type == WP_NUMBER)
        {
            double v =        ((struct wp_node_r*)node)->value
                * wp_ast_eval(((struct wp_node_r*)node)->r);
            ((struct wp_number*)node)->type = WP_NUMBER;
            ((struct wp_number*)node)->value = v;
        }
        break;
    case WP_DIVR:
        wp_ast_optimize(node->r);
        if (node->r->type == WP_NUMBER)
        {
            double v =        ((struct wp_node_r*)node)->value
                / wp_ast_eval(((struct wp_node_r*)node)->r);
            ((struct wp_number*)node)->type = WP_NUMBER;
            ((struct wp_number*)node)->value = v;
        }
        break;
    default:
        yyerror("wp_ast_optimize: unknown node type %d\n", node->type);
        exit(1);
    }
}

void
wp_ast_regvar (struct wp_node* node, char const* name, double* p)
{
    switch (node->type)
    {
    case WP_NUMBER:
        break;
    case WP_SYMBOL:
        if (strcmp(name, ((struct wp_symbol*)node)->name) == 0) {
            ((struct wp_symbol*)node)->pointer = p;
        }
        break;
    case WP_ADD:
    case WP_SUB:
    case WP_MUL:
    case WP_DIV:
        wp_ast_regvar(node->l, name, p);
        wp_ast_regvar(node->r, name, p);
        break;
    case WP_NEG:
        wp_ast_regvar(node->l, name, p);
        break;
    case WP_F1:
        wp_ast_regvar(node->l, name, p);
        break;
    case WP_F2:
        wp_ast_regvar(node->l, name, p);
        wp_ast_regvar(node->r, name, p);
        break;
    case WP_ADDR:
    case WP_SUBR:
    case WP_MULR:
    case WP_DIVR:
        wp_ast_regvar(node->r, name, p);
        break;
    default:
        yyerror("wp_ast_regvar: unknown node type %d\n", node->type);
        exit(1);
    }
}

void wp_ast_setconst (struct wp_node* node, char const* name, double c)
{
    switch (node->type)
    {
    case WP_NUMBER:
        break;
    case WP_SYMBOL:
        if (strcmp(name, ((struct wp_symbol*)node)->name) == 0) {
            ((struct wp_number*)node)->type = WP_NUMBER;
            ((struct wp_number*)node)->value = c;
        }
        break;
    case WP_ADD:
    case WP_SUB:
    case WP_MUL:
    case WP_DIV:
        wp_ast_setconst(node->l, name, c);
        wp_ast_setconst(node->r, name, c);
        break;
    case WP_NEG:
        wp_ast_setconst(node->l, name, c);
        break;
    case WP_F1:
        wp_ast_setconst(node->l, name, c);
        break;
    case WP_F2:
        wp_ast_setconst(node->l, name, c);
        wp_ast_setconst(node->r, name, c);
        break;
    case WP_ADDR:
    case WP_SUBR:
    case WP_MULR:
    case WP_DIVR:
        wp_ast_setconst(node->r, name, c);
        break;
    default:
        yyerror("wp_ast_setconst: unknown node type %d\n", node->type);
        exit(1);
    }
}

void
wp_parser_regvar (struct wp_parser* parser, char const* name, double* p)
{
    wp_ast_regvar(parser->ast, name, p);
}

void
wp_parser_setconst (struct wp_parser* parser, char const* name, double c)
{
    wp_ast_setconst(parser->ast, name, c);
    wp_ast_optimize(parser->ast);
}



