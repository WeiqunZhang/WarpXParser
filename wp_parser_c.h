#ifndef WP_PARSER_C_H_
#define WP_PARSER_C_H_

#ifdef __cplusplus
extern "C" {
#endif

    void* wp_c_parser_new (char const* function_body);
    void  wp_c_parser_delete (void* parser);
    void* wp_c_parser_dup (void* source);

    void wp_c_parser_regvar (void* parser, char const* name, double* p);
    void wp_c_parser_setconst (void* parser, char const* name, double c);

    extern double wp_parser_eval (void* parser);
    inline double wp_c_parser_eval (void* parser) {
        return wp_parser_eval(parser);
    }
    
#ifdef __cplusplus
}
#endif

#endif
