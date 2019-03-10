#ifndef WP_PARSER_C_H_
#define WP_PARSER_C_H_

#ifdef __cplusplus
extern "C" {
#endif

    void* wp_c_parse (char const* body);

    double wp_c_eval (void* parser, double const* dp);

    void wp_c_finalize (void* parser);

#ifdef __cplusplus
}
#endif

#endif
