
#include <AMReX.H>
#include <AMReX_Print.H>
#include <AMReX_ParmParse.H>
#include <AMReX_TinyProfiler.H>
#include <AMReX_FArrayBox.H>
#include "WarpXParser.H"

using namespace amrex;

int main(int argc, char* argv[])
{
    amrex::Initialize(argc,argv);
    {
        ParmParse pp;
        std::string expr;
        pp.get("f", expr);

        std::vector<std::string> constant_names;
        std::vector<double>      constant_values;

        pp.queryarr("constant_names", constant_names);
        pp.queryarr("constant_values", constant_values);

        const int ncells = 100;
        const double dx = 1.0/ncells;
        FArrayBox fab{Box{IntVect{0},IntVect{ncells-1}},1};
        fab.setVal(0.0);
        const auto& a = fab.array();
        const auto lo = lbound(fab.box());
        const auto hi = ubound(fab.box());

        // WarpXParser must be constructed outside OpenMP region
        WarpXParser parser(expr);

        // Constans can be set outside OpenMP parallel region.
        // Note that a constant can only be set once, and the effect
        // is irreversible.
        for (int i = 0; i < constant_names.size(); ++i) {
            parser.setConstant(constant_names[i], constant_values[i]);
        }

        {
            TinyProfiler tp("parser");
#ifdef _OPENMP
#pragma omp parallel
#endif
            {
                // All OpenMP threads need to register variables.
                // These variables must outlive `eval()`,
                // and they are private to avoid race conditions.
                // Call registerVariable outside for loops because they
                // only need to be registered once.
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
    }
    amrex::Finalize();
}

