
#include "WarpXParser.H"
#include "wp_parser_c.h"
#include <AMReX_Print.H>

WarpXParser::WarpXParser (std::string func_body, std::string func_parms)
    : m_body(func_body),
      m_parms(func_parms)
{
    std::string f = "lambda(" + m_parms + ")=" + m_body + "\n";
    m_parser = wp_c_parse(f.c_str());
}

WarpXParser::~WarpXParser ()
{
    wp_c_finalize(m_parser);
}

double
WarpXParser::eval (double const* args)
{
    return wp_c_eval(m_parser, args);
}
