
#include <AMReX.H>
#include <AMReX_Print.H>
#include <AMReX_ParmParse.H>
#include <AMReX_TinyProfiler.H>
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
        
        std::string var = "x,y,z";

        const int nm = 1;
        const int nk = 100;
        const int nj = 10;
        const int ni = 10;
        Vector<Real> d(nm*nk*nj*ni, 0.0);

        // Fortran
        int parser_instance_number = parser_initialize_function(expr.c_str(), expr.length(),
                                                                var.c_str(), var.length());
        {
            TinyProfiler tp("F");
            Real xyz[3];
            for (int m = 0; m < nm; ++m) {
                Real t = m*1.e-10;
                for (int k = 0; k < nk; ++k) {
                    xyz[2] = (k+0.5)*1.e-6;
                    for (int j = 0; j < nj; ++j) {
                        xyz[1] = (j+0.5)*1.e-6;
                        for (int i = 0; i < ni; ++i) {
                            xyz[0] = (i+0.5)*1.e-6;
                            size_t pos = i + j*ni + k*(ni*nj) + m*static_cast<size_t>(ni*nj*nk);
                            d[pos] = parser_evaluate_function(xyz, 3, parser_instance_number);
                        }
                    }
                }
            }
        }
            
        Real tot = 0.0;
        for (Real t : d) {
            tot += t;
        }
        amrex::Print().SetPrecision(17) << " F Total is " << tot << "\n";

        // C
        WarpXParser parser(expr, var);
        {
            TinyProfiler tp("C");
            for (int m = 0; m < nm; ++m) {
                Real t = m*1.e-10;
#ifdef _OPENMP
#pragma omp parallel
#endif
              {
                Real xyz[3];                
#ifdef _OPENMP
#pragma omp for
#endif
                for (int k = 0; k < nk; ++k) {
                    xyz[2] = (k+0.5)*1.e-6;
                    for (int j = 0; j < nj; ++j) {
                        xyz[1] = (j+0.5)*1.e-6;
                        for (int i = 0; i < ni; ++i) {
                            xyz[0] = (i+0.5)*1.e-6;
                            size_t pos = i + j*ni + k*(ni*nj) + m*static_cast<size_t>(ni*nj*nk);
                            d[pos] = parser.eval(xyz);
                        }
                    }
                }
              }
            }
        }
            
        tot = 0.0;
        for (Real t : d) {
            tot += t;
        }
        amrex::Print().SetPrecision(17) << " C Total is " << tot << "\n";
    }
    amrex::Finalize();
}

