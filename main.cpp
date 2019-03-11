
#include <AMReX.H>
#include <AMReX_Print.H>
#include <AMReX_ParmParse.H>
#include <AMReX_TinyProfiler.H>
#include <AMReX_FArrayBox.H>
#include "WarpXParser.H"

using namespace amrex;

extern "C" {
    int parser_initialize_function(const char *str_func, const int len_func,
                                   const char *str_var, const int len_var);
    amrex::Real parser_evaluate_function(const amrex::Real*, const int, const int);
}

int main(int argc, char* argv[])
{
    amrex::Initialize(argc,argv);
    {
        ParmParse pp;
        std::string expr;
        pp.get("f", expr);

        int use_fortran_parser = 1;
        pp.query("use_fortran_parser", use_fortran_parser);
        
        std::string var = "x,y,z";

        const int ncells = 100;
        const double dx = 1.0/ncells;
        FArrayBox fab{Box{IntVect{0},IntVect{ncells-1}},1};
        fab.setVal(0.0);
        const auto& a = fab.array();
        const auto lo = lbound(fab.box());
        const auto hi = ubound(fab.box());

        if (use_fortran_parser)
        {
            // Fortran
            int parser_instance_number = parser_initialize_function(expr.c_str(), expr.length(),
                                                                    var.c_str(), var.length());
            {
                TinyProfiler tp("F");
                Real xyz[3];
                for         (int k = lo.z; k <= hi.z; ++k) {
                    xyz[2]         = (k+0.5)*dx - 0.5;
                    for     (int j = lo.y; j <= hi.y; ++j) {
                        xyz[1]     = (j+0.5)*dx - 0.5;
                        for (int i = lo.x; i <= hi.x; ++i) {
                            xyz[0] = (i+0.5)*dx - 0.5;
                            a(i,j,k) = parser_evaluate_function(xyz, 3, parser_instance_number);
                        }
                    }
                }
            }
            
            amrex::Print().SetPrecision(17) << " F Total is " << fab.sum(0) << "\n";
        }

        {
            // C
            WarpXParser parser(expr);
            {
                TinyProfiler tp("C");
#ifdef _OPENMP
#pragma omp parallel
#endif
                {
                    Real x, y, z;
                    parser.registerVariable("x",x);
                    parser.registerVariable("y",y);
                    parser.registerVariable("z",z);
#ifdef _OPENMP
#pragma omp for
#endif
                    for         (int k = lo.z; k <= hi.z; ++k) {
                        z         = (k+0.5)*dx - 0.5;
                        for     (int j = lo.y; j <= hi.y; ++j) {
                            y     = (j+0.5)*dx - 0.5;
                            for (int i = lo.x; i <= hi.x; ++i) {
                                x = (i+0.5)*dx - 0.5;
                                a(i,j,k) = parser.eval();
                            }
                        }
                    }
                }
            }
            
            amrex::Print().SetPrecision(17) << " C Total is " << fab.sum(0) << "\n";
        }
    }
    amrex::Finalize();
}

