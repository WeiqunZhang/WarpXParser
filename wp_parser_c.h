#ifndef WP_PARSER_C_H_
#define WP_PARSER_C_H_

#include <AMReX_Extension.H>

#ifdef __cplusplus
extern "C" {
#endif

    double wp_parser_eval (void* parser, double const* dp);

    void* wp_c_parse (char const* body);

    void* wp_c_parserdup (void* psrc);

    AMREX_INLINE double wp_c_eval (void* parser, double const* dp) {
        return wp_parser_eval(parser, dp);
    }

    void wp_c_finalize (void* parser);

#ifdef __cplusplus
}
#endif

#endif
