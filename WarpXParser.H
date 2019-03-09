#ifndef WARPX_PARSER_H
#define WARPX_PARSER_H

#include <vector>
#include <string>

class WarpXParser
{
public:
    WarpXParser (std::string func_body, std::string func_parms);
    ~WarpXParser ();

    double eval (double const* args);

private:
    std::string m_body;
    std::string m_parms;
    void* m_parser;
};

#endif
