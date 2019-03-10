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
    WP_POW = 1,
    WP_GT,
    WP_LT
};

enum wp_node_t {
    WP_NUM = 1,
    WP_LIST = 2,
    WP_SYMREF = 3,
    WP_ADD = 4,
    WP_SUB = 5,
    WP_MUL = 6,
    WP_DIV = 7,
    WP_NEG = 8,
    WP_F1 = 9,
    WP_F2 = 10,
    WP_LAMBDA = 11
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

#define WP_MAXLEN_NAME 8

struct wp_symbol {
    char name[WP_MAXLEN_NAME];
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

void wp_freesymlist (struct wp_symlist* symlist);

struct wp_symbol* wp_lookup (char* name);

extern int yylineno; /* from lexer */
void yyerror (char *s, ...);

/*******************************************************************/

struct wp_parser {
    void* p_root;
    void* p_free;
    struct wp_symbol* args;
    struct wp_node* ast;
    int nargs;
    size_t sz_mempool;
};
void wp_parser_init (struct wp_parser* my_parser, size_t N, struct wp_symlist* args);
void wp_parser_finalize (void* parser);
void* wp_parser_allocate (struct wp_parser* my_parser, size_t N);

double wp_parser_eval (void* parser, double const* da);
void* wp_parser_optimize (void);
void* wp_parser_dup (void* orig_parser);
size_t wp_parser_size (struct wp_node* node);
struct wp_node* wp_parser_astdup (struct wp_parser* parser, struct wp_node* src, int move);
struct wp_symbol* wp_parser_lookup (struct wp_parser* parser, char* name);

#endif
