#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "wp_parser_y.h"
#include "wp_parser.tab.h"

static struct wp_lambda* wp_lambda_function = 0;
#define WP_NSYMBOLS 16
static struct wp_symbol wp_sympool[WP_NSYMBOLS];

double
wp_evallambda (struct wp_node* a_args)
{
    struct wp_symlist* f_args  = wp_lambda_function->args;
    while (f_args != NULL) {
        if (a_args->type == WP_LIST) {
            f_args->sym->value = wp_eval(a_args->l);
            a_args = a_args->r;
        } else {
            f_args->sym->value = wp_eval(a_args);
        }
        f_args = f_args->next;
    }
    return wp_eval(wp_lambda_function->body);
}

double
wp_eval (struct wp_node* node)
{
    double result;

    switch (node->type)
    {
    case WP_NUM:
        result = ((struct wp_num*)node)->value;
        break;
    case WP_SYMREF:
        result = ((struct wp_symref*)node)->s->value;
        break;
    case WP_ADD:
        result = wp_eval(node->l) + wp_eval(node->r);
        break;
    case WP_SUB:
        result = wp_eval(node->l) - wp_eval(node->r);
        break;
    case WP_MUL:
        result = wp_eval(node->l) * wp_eval(node->r);
        break;
    case WP_DIV:
        result = wp_eval(node->l) / wp_eval(node->r);
        break;
    case WP_NEG:
        result = -wp_eval(node->l);
        break;
    case WP_F1:
        result = wp_callf1((struct wp_f1*)node);
        break;
    case WP_F2:
        result = wp_callf2((struct wp_f2*)node);
        break;
    default:
        printf("Error in eval, node type %d\n", node->type);
        exit(1);
    }

    return result;
}

double
wp_callf1 (struct wp_f1* f1)
{
    double a = wp_eval(f1->l);
    switch (f1->ftype) {
    case WP_SQRT:   return sqrt(a);
    case WP_EXP:    return exp(a);
    case WP_LOG:    return log(a);
    case WP_LOG10:  return log10(a);
    case WP_SIN:    return sin(a);
    case WP_COS:    return cos(a);
    case WP_TAN:    return tan(a);
    case WP_ASIN:   return asin(a);
    case WP_ACOS:   return acos(a);
    case WP_ATAN:   return atan(a);
    case WP_SINH:   return sinh(a);
    case WP_COSH:   return cosh(a);
    case WP_TANH:   return tanh(a);
    case WP_ABS:    return fabs(a);
    default:
        yyerror("Unknow function %d", f1->ftype);
        return 0.0;
    }
}

double
wp_callf2 (struct wp_f2* f2)
{
    double a = wp_eval(f2->l);
    double b = wp_eval(f2->r);
    switch (f2->ftype) {
    case WP_POW:
        return pow(a,b);
    case WP_GT:
        return (a > b) ? 1.0 : 0.0;
    case WP_LT:
        return (a < b) ? 1.0 : 0.0;
    default:
        yyerror("Unknow function %d", f2->ftype);
        return 0.0;
    }
}

void
wp_deflambda (struct wp_symlist* args, struct wp_node* body)
{
    wp_lambda_function = malloc(sizeof(struct wp_lambda));
    wp_lambda_function->type = WP_LAMBDA;
    wp_lambda_function->args = args;
    wp_lambda_function->body = body;
}

struct wp_node*
wp_newnum (double d)
{
    struct wp_num* r = malloc(sizeof(struct wp_num));
    r->type = WP_NUM;
    r->value = d;
    return (struct wp_node*) r;
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

struct wp_node*
wp_newsymref (struct wp_symbol* s)
{
    struct wp_symref* p = malloc(sizeof(struct wp_symref));
    p->type = WP_SYMREF;
    p->s = s;
    return (struct wp_node*)p;
}

struct wp_symlist*
wp_newsymlist (struct wp_symbol* sym, struct wp_symlist* next)
{
    struct wp_symlist* p = malloc(sizeof(struct wp_symlist));
    p->sym = sym;
    p->next = next;
    return p;
}

void
wp_freesymlist (struct wp_symlist* p)
{
    if (p->next != NULL) {
        wp_freesymlist(p->next);
    }
    free(p);
}

struct wp_symbol*
wp_lookup (char* name)
{
    for (int i = 0; i < WP_NSYMBOLS; ++i) {
        if (strlen(wp_sympool[i].name) == 0) {
            strncpy(wp_sympool[i].name, name, WP_MAXLEN_NAME-1);
            wp_sympool[i].name[WP_MAXLEN_NAME-1] = '\0';
            return wp_sympool + i;
        } else if (strncmp(name, wp_sympool[i].name, WP_MAXLEN_NAME-1) == 0) {
            return wp_sympool + i;
        }
    }
    yyerror("Unknown symbol", name);
    return 0;
}

void
yyerror (char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    fprintf(stderr, "Error at line %d:", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

#if 0
int
main (int argc, char* argv[])
{
    printf("> ");
    return yyparse();
}
#endif

/*******************************************************************/

double
wp_parser_eval (void* parser, double const* da)
{
    struct wp_parser* my_parser = (struct wp_parser*)parser;
    for (int i = 0; i < my_parser->nargs; ++i) {
        my_parser->args[i].value = da[i];
    }
    return wp_eval(my_parser->ast);
}

void*
wp_parser_new (void)
{
    struct wp_parser* my_parser = malloc(sizeof(struct wp_parser));

    size_t N_ast = wp_parser_size(wp_lambda_function->body);

    wp_parser_init(my_parser, N_ast, wp_lambda_function->args);

    my_parser->ast = wp_parser_astdup(my_parser, wp_lambda_function->body, 1);

    wp_freesymlist(wp_lambda_function->args);
    free(wp_lambda_function);

    return my_parser;
}

void
wp_parser_optimize (void* parser)
{
    struct wp_parser* my_parser = (struct wp_parser*)parser;
    wp_parser_astopt(my_parser->ast);
}

void
wp_parser_astopt (struct wp_node* node)
{
    // no need to free because they are allocated from the pool
    switch (node->type)
    {
    case WP_NUM:
    case WP_SYMREF:
        break;
    case WP_ADD:
        wp_parser_astopt(node->l);
        wp_parser_astopt(node->r);
        if (node->l->type == WP_NUM &&
            node->r->type == WP_NUM)
        {
            double v = wp_eval(node);
            ((struct wp_num*)node)->type = WP_NUM;
            ((struct wp_num*)node)->value = v;
        }
        break;
    case WP_SUB:
        wp_parser_astopt(node->l);
        wp_parser_astopt(node->r);
        if (node->l->type == WP_NUM &&
            node->r->type == WP_NUM)
        {
            double v = wp_eval(node);
            ((struct wp_num*)node)->type = WP_NUM;
            ((struct wp_num*)node)->value = v;
        }
        break;
    case WP_MUL:
        wp_parser_astopt(node->l);
        wp_parser_astopt(node->r);
        if (node->l->type == WP_NUM &&
            node->r->type == WP_NUM)
        {
            double v = wp_eval(node);
            ((struct wp_num*)node)->type = WP_NUM;
            ((struct wp_num*)node)->value = v;
        }
        break;
    case WP_DIV:
        wp_parser_astopt(node->l);
        wp_parser_astopt(node->r);
        if (node->l->type == WP_NUM &&
            node->r->type == WP_NUM)
        {
            double v = wp_eval(node);
            ((struct wp_num*)node)->type = WP_NUM;
            ((struct wp_num*)node)->value = v;
        }
        break;
    case WP_NEG:
        wp_parser_astopt(node->l);
        if (node->l->type == WP_NUM)
        {
            double v = wp_eval(node);
            ((struct wp_num*)node)->type = WP_NUM;
            ((struct wp_num*)node)->value = v;
        }
        break;
    case WP_F1:
        wp_parser_astopt(node->l);
        if (node->l->type == WP_NUM)
        {
            double v = wp_eval(node);
            ((struct wp_num*)node)->type = WP_NUM;
            ((struct wp_num*)node)->value = v;
        }
        break;
    case WP_F2:
        wp_parser_astopt(node->l);
        wp_parser_astopt(node->r);
        if (node->l->type == WP_NUM &&
            node->r->type == WP_NUM)
        {
            double v = wp_eval(node);
            ((struct wp_num*)node)->type = WP_NUM;
            ((struct wp_num*)node)->value = v;
        }
        break;
    default:
        printf("Error in wp_astopt, node type %d\n", node->type);
        exit(1);
    }
}

void*
wp_parser_dup (void* a_orig_parser)
{
    struct wp_parser* orig_parser = (struct wp_parser*)a_orig_parser;

    struct wp_parser* my_parser = malloc(sizeof(struct wp_parser));
    my_parser->sz_mempool = orig_parser->sz_mempool;
    my_parser->p_root = malloc(my_parser->sz_mempool);
    my_parser->p_free = my_parser->p_root;

    my_parser->nargs = orig_parser->nargs;
    my_parser->args = wp_parser_allocate(my_parser, sizeof(struct wp_symref)*my_parser->nargs);
    for (int i = 0; i < my_parser->nargs; ++i) {
        strcpy(my_parser->args[i].name, orig_parser->args[i].name);
    }

    my_parser->ast = wp_parser_astdup(my_parser, orig_parser->ast, 0);

    return my_parser;
}

static size_t
wp_aligned_size (size_t N)
{
    const unsigned int align_size = 16;
    size_t x = N + (align_size-1);
    x -= x & (align_size-1);
    return x;
}

size_t
wp_parser_size (struct wp_node* node)
{
    size_t result;

    switch (node->type)
    {
    case WP_NUM:
        result = wp_aligned_size(sizeof(struct wp_num));
        break;
    case WP_SYMREF:
        result = wp_aligned_size(sizeof(struct wp_symref));
        break;
    case WP_ADD:
    case WP_SUB:
    case WP_MUL:
    case WP_DIV:
        result = wp_aligned_size(sizeof(struct wp_node))
            + wp_parser_size(node->l) + wp_parser_size(node->r);
        break;
    case WP_NEG:
        result = wp_aligned_size(sizeof(struct wp_node))
            + wp_parser_size(node->l);
        break;
    case WP_F1:
        result = wp_aligned_size(sizeof(struct wp_f1))
            + wp_parser_size(node->l);
        break;
    case WP_F2:
        result = wp_aligned_size(sizeof(struct wp_f2))
            + wp_parser_size(node->l) + wp_parser_size(node->r);
        break;
    default:
        printf("Error in count_size, node type %d\n", node->type);
        exit(1);
    }

    return result;
}

struct wp_node*
wp_parser_astdup (struct wp_parser* my_parser, struct wp_node* node, int move)
{
    void* result;

    switch (node->type)
    {
    case WP_NUM:
        result = wp_parser_allocate(my_parser, sizeof(struct wp_num));
        ((struct wp_num*)result)->type = node->type;
        ((struct wp_num*)result)->value = ((struct wp_num*)node)->value;
        break;
    case WP_SYMREF:
        result = wp_parser_allocate(my_parser, sizeof(struct wp_symref));
        ((struct wp_symref*)result)->type = node->type;
        ((struct wp_symref*)result)->s = wp_parser_lookup(my_parser, ((struct wp_symref*)node)->s->name);
        break;
    case WP_ADD:
    case WP_SUB:
    case WP_MUL:
    case WP_DIV:
        result = wp_parser_allocate(my_parser, sizeof(struct wp_node));
        ((struct wp_node*)result)->type = node->type;
        ((struct wp_node*)result)->l = wp_parser_astdup(my_parser, node->l, move);
        ((struct wp_node*)result)->r = wp_parser_astdup(my_parser, node->r, move);
        break;
    case WP_NEG:
        result = wp_parser_allocate(my_parser, sizeof(struct wp_node));
        ((struct wp_node*)result)->type = node->type;
        ((struct wp_node*)result)->l = wp_parser_astdup(my_parser, node->l, move);
        ((struct wp_node*)result)->r = NULL;
        break;
    case WP_F1:
        result = wp_parser_allocate(my_parser, sizeof(struct wp_f1));
        ((struct wp_f1*)result)->type = node->type;
        ((struct wp_f1*)result)->l = wp_parser_astdup(my_parser, ((struct wp_f1*)node)->l, move);
        ((struct wp_f1*)result)->ftype = ((struct wp_f1*)node)->ftype;
        break;
    case WP_F2:
        result = wp_parser_allocate(my_parser, sizeof(struct wp_f2));
        ((struct wp_f2*)result)->type = node->type;
        ((struct wp_f2*)result)->l = wp_parser_astdup(my_parser, ((struct wp_f2*)node)->l, move);
        ((struct wp_f2*)result)->r = wp_parser_astdup(my_parser, ((struct wp_f2*)node)->r, move);
        ((struct wp_f2*)result)->ftype = ((struct wp_f2*)node)->ftype;
        break;
    default:
        printf("Error in eval, node type %d\n", node->type);
        exit(1);
    }
    if (move) free((void*)node);
    return result;
}

void
wp_parser_init (struct wp_parser* my_parser, size_t N_ast, struct wp_symlist* args)
{
    int nargs = 0;
    struct wp_symlist* f_args = args;
    while (f_args != NULL) {
        f_args = f_args->next;
        ++nargs;
    }

    my_parser->sz_mempool = N_ast + wp_aligned_size(sizeof(struct wp_symbol))*nargs;
    my_parser->p_root = malloc(my_parser->sz_mempool);
    my_parser->p_free = my_parser->p_root;

    my_parser->args = wp_parser_allocate(my_parser, sizeof(struct wp_symref)*nargs);
    f_args = args;
    for (int i = 0; i < nargs; ++i) {
        strcpy(my_parser->args[i].name, f_args->sym->name);
        f_args = f_args->next;
    }
    my_parser->nargs = nargs;
}

void
wp_parser_finalize (void* parser)
{
    struct wp_parser* my_parser = (struct wp_parser*)parser;
    free(my_parser->p_root);
    free(my_parser);
}

void*
wp_parser_allocate (struct wp_parser* my_parser, size_t N)
{
    void* r = my_parser->p_free;
    my_parser->p_free = (char*)r + wp_aligned_size(N);
    return r;
}

struct wp_symbol*
wp_parser_lookup (struct wp_parser* my_parser, char* name)
{
    for (int i = 0; i < my_parser->nargs; ++i) {
        if (strcmp(name, my_parser->args[i].name) == 0) {
            return my_parser->args+i;
        }
    }
    return NULL;
}
