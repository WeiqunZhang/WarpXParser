
#include "WarpXParser.H"
#include "wp_parser_c.h"
#include <AMReX_Print.H>

#ifdef _OPENMP
#include <omp.h>
#endif

WarpXParser::WarpXParser (std::string func_body, std::string func_parms)
    : m_body(func_body),
      m_parms(func_parms)
{
#ifdef _OPENMP
    int nthreads = omp_get_max_threads();
#else
    int nthreads = 1;
#endif
    m_parser.resize(nthreads);

    std::string f = "lambda(" + m_parms + ")=" + m_body + "\n";
    m_parser[0] = wp_c_parse(f.c_str());

#ifdef _OPENMP
#pragma omp parallel
    {
        int tid = omp_get_thread_num();
        if (tid > 0) {
            m_parser[tid] = wp_c_parserdup(m_parser[0]);
        }
    }
#endif
}

WarpXParser::~WarpXParser ()
{
#ifdef _OPENMP
#pragma omp parallel
    {
        int tid = omp_get_thread_num();
        wp_c_finalize(m_parser[tid]);
    }
#else
    wp_c_finalize(m_parser[0]);
#endif
}

double
WarpXParser::eval (double const* args)
{
#ifdef _OPENMP
    int tid = omp_get_thread_num();
#else
    int tid = 0;
#endif
    return wp_c_eval(m_parser[tid], args);
}
