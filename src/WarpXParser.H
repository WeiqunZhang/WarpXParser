#ifndef WARPX_PARSER_H_
#define WARPX_PARSER_H_

#include <vector>
#include <string>

#include "wp_parser_c.h"
#include "wp_parser_y.h"

#ifdef _OPENMP
#include <omp.h>
#endif


class WarpXParser
{
private:
    std::vector<struct wp_parser*> m_parser;
    wp_parser* p0;

public:
    WarpXParser (std::string const& func_body);
    ~WarpXParser ();

    void registerVariable (std::string const& name, double& var);
    void setConstant (std::string const& name, double c);

    inline double eval () {
#ifdef _OPENMP
        return wp_ast_eval(m_parser[omp_get_thread_num()]->ast);
#else
        return wp_ast_eval(p0->ast);
#endif
    }
};

#endif
