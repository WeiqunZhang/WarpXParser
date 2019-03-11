
#include "WarpXParser.H"

WarpXParser::WarpXParser (std::string const& func_body)
{
#ifdef _OPENMP
    int nthreads = omp_get_max_threads();
#else
    int nthreads = 1;
#endif
    m_parser.resize(nthreads);

    std::string f = func_body + "\n";
    m_parser[0] = wp_c_parser_new(f.c_str());

#ifdef _OPENMP
#pragma omp parallel
    {
        int tid = omp_get_thread_num();
        if (tid > 0) {
            m_parser[tid] = wp_parser_dup(m_parser[0]);
        }
    }
#endif

    p0 = m_parser[0];
}

WarpXParser::~WarpXParser ()
{
#ifdef _OPENMP
#pragma omp parallel
    {
        int tid = omp_get_thread_num();
        wp_parser_delete(m_parser[tid]);
    }
#else
    wp_parser_delete(p0);
#endif
}

void
WarpXParser::registerVariable (std::string const& name, double& var)
{
    // We assume this is called inside OMP parallel region
#ifdef _OPENMP
    wp_parser_regvar(m_parser[omp_get_thread_num()], name.c_str(), &var);
#else
    wp_parser_regvar(p0, name.c_str(), &var);
#endif
}

void
WarpXParser::setConstant (std::string const& name, double c)
{
    // We don't know if this is inside OMP parallel region or not
#ifdef _OPENMP
    bool in_parallel = omp_in_parallel();
#pragma omp parallel if (!in_parallel)
    {
        wp_parser_setconst(m_parser[omp_get_thread_num()], name.c_str(), c);
    }
#else
    wp_parser_setconst(p0, name.c_str(), c);
#endif
}

