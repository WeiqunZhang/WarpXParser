#ifndef WP_PARSER_Y_H_
#define WP_PARSER_Y_H_

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
    WP_TANH
};

enum wp_f2_t {  // Built-in functions with two arguments
    WP_POW = 1
};

enum wp_node_t {
    WP_NUM = 1,
    WP_LIST,
    WP_SYMREF,
    WP_ADD,
    WP_SUB,
    WP_MUL,
    WP_DIV,
    WP_NEG,
    WP_F1,
    WP_F2,
    WP_LAMBDA
};

struct wp_node {
    enum wp_node_t type;
    struct wp_node* l;
    struct wp_node* r;
};

struct wp_num {
    enum wp_node_t type;
    double value;
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

struct wp_lambda {
    enum wp_node_t type;
    struct wp_symlist* args;
    struct wp_node* body;
};

struct wp_symbol {
    char name[16];
    double value;
};

struct wp_symlist {
    struct wp_symbol* sym;
    struct wp_symlist* next;
};

struct wp_symref {
    enum wp_node_t type;
    struct wp_symbol* s;
};


double wp_evallambda (struct wp_node* args);
double wp_eval (struct wp_node* node);
double wp_callf1 (struct wp_f1* f1);
double wp_callf2 (struct wp_f2* f2);

void wp_deflambda (struct wp_symlist* parms, struct wp_node* body);

struct wp_node* wp_newnum (double d);
struct wp_node* wp_newnode (enum wp_node_t type, struct wp_node* l, struct wp_node* r);
struct wp_node* wp_newf1 (enum wp_f1_t ftype, struct wp_node* l);
struct wp_node* wp_newf2 (enum wp_f2_t ftype, struct wp_node* l, struct wp_node* r);
struct wp_node* wp_newsymref (struct wp_symbol* s);
struct wp_symlist* wp_newsymlist (struct wp_symbol* sym, struct wp_symlist* next);

struct wp_symbol* wp_lookup (char* name);

extern int yylineno; /* from lexer */
void yyerror (char *s, ...);

/*******************************************************************/

double wp_eval_dp (double const* da);

#endif
