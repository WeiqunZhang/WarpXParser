#ifndef WARPX_PARSER_H_
#define WARPX_PARSER_H_

#include <vector>
#include <string>

#include "wp_parser_c.h"

#ifdef _OPENMP
#include <omp.h>
#endif


class WarpXParser
{
public:
    WarpXParser (std::string const& func_body);
    ~WarpXParser ();

    void registerVariable (std::string const& name, double& var);
    void setConstant (std::string const& name, double c);

    inline double eval () {
#ifdef _OPENMP
        return wp_c_parser_eval(m_parser[omp_get_thread_num()]);
#else
        return wp_c_parser_eval(p0);
#endif
    }

private:
    std::vector<void*> m_parser;
    void* p0;
};

#endif
