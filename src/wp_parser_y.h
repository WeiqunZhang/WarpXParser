#ifndef WP_PARSER_Y_H_
#define WP_PARSER_Y_H_

#ifdef __cplusplus
#include <cstdlib>
extern "C" {
#else
#include <stdlib.h>
#endif

enum wp_f1_t {  // Bulit-in functions with one argument
    WP_SQRT = 1,
    WP_EXP,
    WP_LOG,
    WP_LOG10,
    WP_SIN,
    WP_COS,
    WP_TAN,
    WP_ASIN,
    WP_ACOS,
    WP_ATAN,
    WP_SINH,
    WP_COSH,
    WP_TANH,
    WP_ABS
};

enum wp_f2_t {  // Built-in functions with two arguments
    WP_POW = 1,
    WP_GT,
    WP_LT,
    WP_HEAVISIDE
};

enum wp_node_t {
    WP_NUMBER = 1,
    WP_SYMBOL,
    WP_ADD,
    WP_SUB,
    WP_MUL,
    WP_DIV,
    WP_NEG,
    WP_F1,
    WP_F2
};

struct wp_node {
    enum wp_node_t type;
    struct wp_node* l;
    struct wp_node* r;
};

struct wp_number {
    enum wp_node_t type;
    double value;
};

struct wp_symbol {
    enum wp_node_t type;
    char* name;
    double* pointer;
};

struct wp_f1 {  /* Builtin functions with one argument */
    enum wp_node_t type;
    struct wp_node* l;
    enum wp_f1_t ftype;
};

struct wp_f2 {  /* Builtin functions with two arguments */
    enum wp_node_t type;
    struct wp_node* l;
    struct wp_node* r;
    enum wp_f2_t ftype;
};

/*******************************************************************/

void wp_defexpr (struct wp_node* body);
struct wp_node* wp_newnumber (double d);
struct wp_symbol* wp_makesymbol (char* name);
struct wp_node* wp_newsymbol (struct wp_symbol* sym);
struct wp_node* wp_newnode (enum wp_node_t type, struct wp_node* l,
                            struct wp_node* r);
struct wp_node* wp_newf1 (enum wp_f1_t ftype, struct wp_node* l);
struct wp_node* wp_newf2 (enum wp_f2_t ftype, struct wp_node* l,
                          struct wp_node* r);

void yyerror (char const *s, ...);

/*******************************************************************/

struct wp_parser {
    void* p_root;
    void* p_free;
    struct wp_node* ast;
    size_t sz_mempool;
};

struct wp_parser* wp_parser_new (void);
void wp_parser_delete (struct wp_parser* parser);
void* wp_parser_allocate (struct wp_parser* my_parser, size_t N);

struct wp_parser* wp_parser_dup (struct wp_parser* source);
struct wp_node* wp_parser_ast_dup (struct wp_parser* parser, struct wp_node* src, int move);

double wp_ast_eval (struct wp_node* node);
void wp_ast_optimize (struct wp_node* node);
size_t wp_ast_size (struct wp_node* node);
void wp_ast_regvar (struct wp_node* node, char const* name, double* p);
void wp_ast_setconst (struct wp_node* node, char const* name, double c);

void wp_parser_regvar (struct wp_parser* parser, char const* name, double* p);
void wp_parser_setconst (struct wp_parser* parser, char const* name, double c);

#ifdef __cplusplus
}
#endif

#endif
