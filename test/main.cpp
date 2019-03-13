
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
        std::string expr("x+y");
        pp.query("f", expr);

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
                double x, y, z;
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

        amrex::Print() << "\n";

        {
            WarpXParser parser("a0*x**2 * (1-y*1.e2) * heaviside(x,0.)");
            const double a0 = 3.;
            parser.setConstant("a0", a0);
            double y = 5.7;
            double x;
            parser.registerVariable("x",x);
            parser.registerVariable("y",y);
            bool failed = false;
            for (x = -0.03; x <= 0.03; x += 0.0001) {
                double r = parser.eval();
                double e = a0*x*x * (1.-y*1e2) * (x <=0 ? 0. : 1.);
                if (r != e && std::abs((r-e)/std::max(r,e)) > 1.e-14) {
                    amrex::Print().SetPrecision(17) << "FAIL: " << parser.expr() << ", "
                                                    << r << ", " << e << std::endl;
                    failed = true;
                }
            }
            if (!failed) {
                amrex::Print() << "PASS: " << parser.expr() << std::endl;
            }
        }

        {
            WarpXParser parser("n0+n0*x**2*1.e12");
            const double n0 = 1.345;
            parser.setConstant("n0", n0);
            double x;
            parser.registerVariable("x", x);
            bool failed = false;
            for (x = -0.03; x <= 0.03; x += 0.0001) {
                double r = parser.eval();
                double e = n0+n0*(x*x)*1.e12;
                if (r != e && std::abs((r-e)/std::max(r,e)) > 1.e-14) {
                    amrex::Print().SetPrecision(17) << "FAIL: " << parser.expr() << ", "
                                                    << r << ", " << e << std::endl;
                    failed = true;
                }
            }
            if (!failed) {
                amrex::Print() << "PASS: " << parser.expr() << std::endl;
            }
        }

        {
            WarpXParser parser("a0*x**2 * heaviside(x,0.) * cos(omega0*t)");
            const double a0 = 3.5e-2;
            const double omega0 = 0.45e-1;
            parser.setConstant("a0", a0);
            parser.setConstant("omega0", omega0);
            double x = 4.6;
            double t = 6.65e-5;
            parser.registerVariable("x",x);
            parser.registerVariable("t",t);
            bool failed = false;
            for (t = 6.3e-6; t <= 6.3e-5; t += 1.1e-6) {
                for (x = -0.01; x <= 0.01; x += 0.001) {
                    double r = parser.eval();
                    double e = a0*(x*x) * cos(omega0*t) * (x<=0 ? 0. : 1.);
                    if (r != e && std::abs((r-e)/std::max(r,e)) > 1.e-14) {
                        amrex::Print().SetPrecision(17) << "FAIL: " << parser.expr() << ", "
                                                        << r << ", " << e << std::endl;
                        failed = true;
                    }
                }
            }
            if (!failed) {
                amrex::Print() << "PASS: " << parser.expr() << std::endl;
            }
        }

        {
            WarpXParser parser("((x**2+y**2) < rmax**2) * n0 * (1+4*(x**2+y**2)/(kp**2*rc**4))");
            const double rmax = 110.e-6;
            const double n0 = 3.5e-24;
            const double kp = 353352.;
            const double rc = 50.e-6;
            parser.setConstant("rmax",rmax);
            parser.setConstant("n0",n0);
            parser.setConstant("kp",kp);
            parser.setConstant("rc",rc);
            double x;
            double y;
            parser.registerVariable("x",x);
            parser.registerVariable("y",y);
            bool failed = false;
            for (x = -1.e-4; x <= 1.e-4; x += 1.2e-5) {
                for (y = -2.2e-4; y <= 1.2e-4; y += 2.e-5) {
                    double r = parser.eval();
                    double e = (((x*x+y*y) < rmax*rmax) ? 1.0 : 0.0) * n0 * (1+4*(x*x+y*y)/(kp*kp*rc*rc*rc*rc));
                    if (r != e && std::abs((r-e)/std::max(r,e)) > 1.e-14) {
                        amrex::Print().SetPrecision(17) << "FAIL: " << parser.expr() << ", "
                                                        << r << ", " << e << std::endl;
                        failed = true;
                    }
                }
            }
            if (!failed) {
                amrex::Print() << "PASS: " << parser.expr() << std::endl;
            }
        }

        {
            WarpXParser parser("(z<lramp)*0.5*(1-cos(pi*z/lramp))*dens+(z>lramp)*dens");
            const double lramp = 8.e-3;
            const double pi = 3.14159265358979323846264338327950288419716939937510;
            const double dens = 1.e23;
            parser.setConstant("lramp",lramp);
            parser.setConstant("pi",pi);
            parser.setConstant("dens",dens);
            double z;
            parser.registerVariable("z",z);
            bool failed = false;
            for (z = 6.e-3; z <= 10.e-3; z += 0.45e-4) {
                double r = parser.eval();
                double e = ((z<lramp)?1.0:0.0)*0.5*(1-cos(pi*z/lramp))*dens+((z>lramp)?1.:0.)*dens;
                if (r != e && std::abs((r-e)/std::max(r,e)) > 1.e-14) {
                    amrex::Print().SetPrecision(17) << "FAIL: " << parser.expr() << ", "
                                                    << r << ", " << e << std::endl;
                    failed = true;
                }
            }
            if (!failed) {
                amrex::Print() << "PASS: " << parser.expr() << std::endl;
            }
        }

        {
            WarpXParser parser("(z<zp)*nc*exp((z-zc)/lgrad)+(z>zp)*(z<zp2)*2.*nc+(z>zp2)*nc*exp(-(z-zc2)/lgrad)");
            const double zc = 20.e-6;
            const double zp = 20.05545177444479562e-6;
            const double lgrad = .08e-6;
            const double nc = 1.74e27;
            const double zp2 = 24.e-6;
            const double zc2 = 24.05545177444479562e-6;
            double z;
            parser.setConstant("zc",zc);
            parser.setConstant("zp",zp);
            parser.setConstant("lgrad",lgrad);
            parser.setConstant("nc",nc);
            parser.setConstant("zp2",zp2);
            parser.setConstant("zc2",zc2);
            parser.registerVariable("z",z);
            bool failed = false;
            for (z = 18.0e-6; z <= 22.e-6; z += 1.e-7) {
                double r = parser.eval();
                double e = ((z<zp)?1.:0.)*nc*exp((z-zc)/lgrad)+((z>zp)?1.:0.)*((z<zp2)?1.:0.)*2.*nc+((z>zp2)?1.:0.)*nc*exp(-(z-zc2)/lgrad);
                if (r != e && std::abs((r-e)/std::max(r,e)) > 1.e-14) {
                    amrex::Print().SetPrecision(17) << "FAIL: " << parser.expr() << ", "
                                                    << r << ", " << e << std::endl;
                    failed = true;
                }
            }
            if (!failed) {
                amrex::Print() << "PASS: " << parser.expr() << std::endl;
            }
        }

        {
            WarpXParser parser("epsilon * k/kp * sin(k*x) * cos(k*y) * cos(k*z)");
            const double epsilon = 0.01;
            const double kp = 376357.71524190728;
            const double k = 314159.2653589793;
            parser.setConstant("epsilon",epsilon);
            parser.setConstant("kp",kp);
            parser.setConstant("k",k);
            double x, y, z;
            parser.registerVariable("x",x);
            parser.registerVariable("y",y);
            parser.registerVariable("z",z);
            bool failed = false;
            for         (x = -20.e-6; x <= 20.e-6; x += 0.450e-6) {
                for     (y = -20.e-6; y <= 20.e-6; y += 0.345e-6) {
                    for (z = -20.e-6; z <= 20.e-6; z += 0.245e-6) {
                        double r = parser.eval();
                        double e = epsilon * k/kp * sin(k*x) * cos(k*y) * cos(k*z);
                        if (r != e && std::abs((r-e)/std::max(r,e)) > 1.e-14) {
                            amrex::Print().SetPrecision(17) << "FAIL: " << parser.expr() << ", "
                                                            << r << ", " << e << std::endl;
                            failed = true;
                        }
                    }
                }
            }
            if (!failed) {
                amrex::Print() << "PASS: " << parser.expr() << std::endl;
            }
        }

        {
            WarpXParser parser("epsilon * k/kp * cos(k*x) * sin(k*y) * cos(k*z)");
            const double epsilon = 0.01;
            const double kp = 376357.71524190728;
            const double k = 314159.2653589793;
            parser.setConstant("epsilon",epsilon);
            parser.setConstant("kp",kp);
            parser.setConstant("k",k);
            double x, y, z;
            parser.registerVariable("x",x);
            parser.registerVariable("y",y);
            parser.registerVariable("z",z);
            bool failed = false;
            for         (x = -20.e-6; x <= 20.e-6; x += 0.450e-6) {
                for     (y = -20.e-6; y <= 20.e-6; y += 0.345e-6) {
                    for (z = -20.e-6; z <= 20.e-6; z += 0.245e-6) {
                        double r = parser.eval();
                        double e = epsilon * k/kp * cos(k*x) * sin(k*y) * cos(k*z);
                        if (r != e && std::abs((r-e)/std::max(r,e)) > 1.e-14) {
                            amrex::Print().SetPrecision(17) << "FAIL: " << parser.expr() << ", "
                                                            << r << ", " << e << std::endl;
                            failed = true;
                        }
                    }
                }
            }
            if (!failed) {
                amrex::Print() << "PASS: " << parser.expr() << std::endl;
            }
        }

        {
            WarpXParser parser("-epsilon * k/kp * cos(k*x) * cos(k*y) * sin(k*z)");
            const double epsilon = 0.01;
            const double kp = 376357.71524190728;
            const double k = 314159.2653589793;
            parser.setConstant("epsilon",epsilon);
            parser.setConstant("kp",kp);
            parser.setConstant("k",k);
            double x, y, z;
            parser.registerVariable("x",x);
            parser.registerVariable("y",y);
            parser.registerVariable("z",z);
            bool failed = false;
            for         (x = -20.e-6; x <= 20.e-6; x += 0.450e-6) {
                for     (y = -20.e-6; y <= 20.e-6; y += 0.345e-6) {
                    for (z = -20.e-6; z <= 20.e-6; z += 0.245e-6) {
                        double r = parser.eval();
                        double e = -epsilon * k/kp * cos(k*x) * cos(k*y) * sin(k*z);
                        if (r != e && std::abs((r-e)/std::max(r,e)) > 1.e-14) {
                            amrex::Print().SetPrecision(17) << "FAIL: " << parser.expr() << ", "
                                                            << r << ", " << e << std::endl;
                            failed = true;
                        }
                    }
                }
            }
            if (!failed) {
                amrex::Print() << "PASS: " << parser.expr() << std::endl;
            }
        }
    }
    amrex::Finalize();
}

