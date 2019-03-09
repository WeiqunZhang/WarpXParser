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

struct wp_symbol*
wp_lookup (char* name)
{
    for (int i = 0; i < WP_NSYMBOLS; ++i) {
        if (strlen(wp_sympool[i].name) == 0) {
            strcpy(wp_sympool[i].name, name);
            return wp_sympool + i;
        } else if (strcmp(name, wp_sympool[i].name) == 0) {
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
wp_eval_dp (double const* da)
{
    struct wp_symlist* f_args = wp_lambda_function->args;
    while (f_args != NULL) {
        f_args->sym->value = *da++;
        f_args = f_args->next;
    }
    return wp_eval(wp_lambda_function->body);
}
