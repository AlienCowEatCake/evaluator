#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#if defined(_WIN32)
    #if !defined(NOMINMAX)
        #define NOMINMAX
    #endif
    #include <windows.h>
#elif defined(__MACH__)
    #include <mach/mach_time.h>
#endif
#include "evaluator/evaluator.h"

namespace {

class teestream
{
public:
    teestream(const std::string & filename)
        : m_ofs(filename.c_str(), std::ios::out)
    {}

    template<typename T>
    teestream & operator << (const T & something)
    {
        std::cout << something;
        m_ofs << something;
        return * this;
    }

    typedef std::ostream & (* stream_function)(std::ostream &);
    teestream & operator << (stream_function func)
    {
        func(std::cout);
        func(m_ofs);
        return * this;
    }

private:
    std::fstream m_ofs;
};

unsigned long mtime();
double rand_uniform(double a, double b);
void types_test(teestream & tee);
void self_test(teestream & tee);
void benchmark1(std::size_t num_tests, teestream & tee);

} // namespace

int main(int argc, char * argv[])
{
    std::size_t num_tests = 50000000;
    if(argc > 1)
        num_tests = static_cast<std::size_t>(atoi(argv[1]));

    std::stringstream data, name;
    name << "result_";

#if defined(__GNUC__) && !defined(__clang__)
    data << "Compiler: GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << std::endl;
    name << "gcc-" << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "_";
#endif
#if defined(__clang__)
    data << "Compiler: Clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__ << std::endl;
    name << "clang-" << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__ << "_";
#endif
#if defined(_MSC_VER)
    data << "Compiler: VS " << _MSC_VER << " (" << _MSC_FULL_VER << ")" << std::endl;
    name << "vs-" << _MSC_VER << "_";
#endif
#if defined(EVALUATOR_JIT_X64)
    data << "Arch: x64" << std::endl;
    name << "x64_";
#endif
#if defined(EVALUATOR_JIT_X86)
    data << "Arch: x86" << std::endl;
    name << "x86_";
#endif
#if defined(EVALUATOR_JIT_X32)
    data << "Arch: x32" << std::endl;
    name << "x32_";
#endif
#if defined(EVALUATOR_JIT_MSVC_ABI)
    data << "ABI: MSVC" << std::endl;
    name << "abi-msvc";
#endif
#if defined(EVALUATOR_JIT_SYSV_ABI)
    data << "ABI: SYSV" << std::endl;
    name << "abi-sysv";
#endif
#if defined(EVALUATOR_JIT_MINGW_ABI)
    data << "ABI: MinGW" << std::endl;
    name << "abi-mingw";
#endif

    name << ".txt";
    teestream tee(name.str());
    tee << data.str() << "Data Size: " << num_tests << std::endl;
    tee << "\n================================" << std::endl;
    types_test(tee);
    tee << "\n================================" << std::endl;
    self_test(tee);
    tee  << "\n================================" << std::endl;
    benchmark1(num_tests, tee);

#if defined(_WIN32)
    system("pause");
#endif
    return 0;
}

namespace {

void get_all_exprs(std::vector<std::string> & exprs)
{
    exprs.push_back("x+y     ");
    exprs.push_back("x-y     ");
    exprs.push_back("x*y     ");
    exprs.push_back("x/y     ");
    exprs.push_back("x^y     ");
    exprs.push_back("imag(x) ");
    exprs.push_back("real(x) ");
    exprs.push_back("conj(x) ");
    exprs.push_back("arg(x)  ");
    exprs.push_back("sin(x)  ");
    exprs.push_back("cos(x)  ");
    exprs.push_back("tan(x)  ");
    exprs.push_back("asin(x) ");
    exprs.push_back("acos(x) ");
    exprs.push_back("atan(x) ");
    exprs.push_back("sinh(x) ");
    exprs.push_back("cosh(x) ");
    exprs.push_back("tanh(x) ");
    exprs.push_back("asinh(x)");
    exprs.push_back("acosh(x)");
    exprs.push_back("atanh(x)");
    exprs.push_back("log(x)  ");
    exprs.push_back("log2(x) ");
    exprs.push_back("log10(x)");
    exprs.push_back("abs(x)  ");
    exprs.push_back("exp(x)  ");
    exprs.push_back("sqrt(x) ");
}

void benchmark1(std::size_t num_tests, teestream & tee)
{
    std::vector<std::string> exprs;
    get_all_exprs(exprs);

    evaluator<float> pf;
    evaluator<double> pd;
    evaluator<std::complex<float> > pcf;
    evaluator<std::complex<double> > pcd;

    for(std::size_t i = 0; i < exprs.size(); i++)
    {
        tee << "Test: " << exprs[i] << std::endl;
        tee << "float\tdouble\tcfoat\tcdouble" << std::endl;

        if(!pf.parse(exprs[i]))  std::cout << pf.get_error() << std::endl;
        if(!pd.parse(exprs[i]))  std::cout << pd.get_error() << std::endl;
        if(!pcf.parse(exprs[i])) std::cout << pcf.get_error() << std::endl;
        if(!pcd.parse(exprs[i])) std::cout << pcd.get_error() << std::endl;

        double xd = rand_uniform(0, 1);
        float xf = static_cast<float>(xd);
        double yd = rand_uniform(0, 1);
        float yf = static_cast<float>(yd);
        std::complex<double> xcd = std::complex<double>(xd, yd);
        std::complex<double> ycd = std::complex<double>(yd, xd);
        std::complex<float> xcf = std::complex<float>(xf, yf);
        std::complex<float> ycf = std::complex<float>(yf, xf);
        double rd;
        float rf;
        std::complex<double> rcd;
        std::complex<float> rcf;

        if(exprs[i] == "acosh(x)")
        {
            xd += 1.0;
            xf += 1.0f;
        }

        pf.set_var("x", xf);
        pf.set_var("y", yf);
        pd.set_var("x", xd);
        pd.set_var("y", yd);
        pcf.set_var("x", xcf);
        pcf.set_var("y", ycf);
        pcd.set_var("x", xcd);
        pcd.set_var("y", ycd);

        tee << "--- Extcall: ---" << std::endl;

        if(!pf.compile_extcall())  std::cout << pf.get_error() << std::endl;
        if(!pd.compile_extcall())  std::cout << pd.get_error() << std::endl;
        if(!pcf.compile_extcall()) std::cout << pcf.get_error() << std::endl;
        if(!pcd.compile_extcall()) std::cout << pcd.get_error() << std::endl;

        for(std::size_t j = 0; j < 3; j++)
        {
            unsigned long t;
            t = mtime();
            for(std::size_t k = 0; k < num_tests; k++)
                pf.calculate(rf);
            t = mtime() - t;
            tee << t << "\t";
            t = mtime();
            for(std::size_t k = 0; k < num_tests; k++)
                pd.calculate(rd);
            t = mtime() - t;
            tee << t << "\t";
            t = mtime();
            for(std::size_t k = 0; k < num_tests; k++)
                pcf.calculate(rcf);
            t = mtime() - t;
            tee << t << "\t";
            t = mtime();
            for(std::size_t k = 0; k < num_tests; k++)
                pcd.calculate(rcd);
            t = mtime() - t;
            tee << t << std::endl;
        }

        tee << "--- Inline: ---" << std::endl;

        if(!pf.compile_inline())  std::cout << pf.get_error() << std::endl;
        if(!pd.compile_inline())  std::cout << pd.get_error() << std::endl;
        if(!pcf.compile_inline()) std::cout << pcf.get_error() << std::endl;
        if(!pcd.compile_inline()) std::cout << pcd.get_error() << std::endl;

        for(std::size_t j = 0; j < 3; j++)
        {
            unsigned long t;
            t = mtime();
            for(std::size_t k = 0; k < num_tests; k++)
                pf.calculate(rf);
            t = mtime() - t;
            tee << t << "\t";
            t = mtime();
            for(std::size_t k = 0; k < num_tests; k++)
                pd.calculate(rd);
            t = mtime() - t;
            tee << t << "\t";
            t = mtime();
            for(std::size_t k = 0; k < num_tests; k++)
                pcf.calculate(rcf);
            t = mtime() - t;
            tee << t << "\t";
            t = mtime();
            for(std::size_t k = 0; k < num_tests; k++)
                pcd.calculate(rcd);
            t = mtime() - t;
            tee << t << std::endl;
        }

        tee << "\n--------------------------------" << std::endl;
    }
}

void types_test(teestream & tee)
{
    tee << "Type detection checks:" << std::endl;
    const int NO    = 0;
    const int YES   = 1;
    const int MAYBE = 2;
#define CHECK_TYPE(TYPE, FLOATING, INTEGER, COMPLEX, FLOAING_COMPLEX, FLOAT, DOUBLE, CFLOAT, CDOUBLE) \
    { \
        std::string ct_type = std::string(#TYPE); \
        ct_type.resize(28, ' '); \
        const bool ct_floating  = (evaluator_internal::is_floating<TYPE>()          ? YES : NO) == FLOATING         || FLOATING == MAYBE; \
        const bool ct_integer   = (evaluator_internal::is_integer<TYPE>()           ? YES : NO) == INTEGER          || INTEGER == MAYBE; \
        const bool ct_complex   = (evaluator_internal::is_complex<TYPE>()           ? YES : NO) == COMPLEX          || COMPLEX == MAYBE; \
        const bool ct_fltcmpl   = (evaluator_internal::is_floating_complex<TYPE>()  ? YES : NO) == FLOAING_COMPLEX  || FLOAING_COMPLEX == MAYBE; \
        const bool ct_float     = (evaluator_internal::is_float<TYPE>()             ? YES : NO) == FLOAT            || FLOAT == MAYBE; \
        const bool ct_double    = (evaluator_internal::is_double<TYPE>()            ? YES : NO) == DOUBLE           || DOUBLE == MAYBE; \
        const bool ct_cfloat    = (evaluator_internal::is_complex_float<TYPE>()     ? YES : NO) == CFLOAT           || CFLOAT == MAYBE; \
        const bool ct_cdouble   = (evaluator_internal::is_complex_double<TYPE>()    ? YES : NO) == CDOUBLE          || CDOUBLE == MAYBE; \
        const bool ct_result = ct_floating && ct_integer && ct_complex && ct_fltcmpl && ct_float && ct_double && ct_cfloat && ct_cdouble; \
        tee << ct_type << (ct_result ? std::string("OK") : std::string("FAIL")) << std::endl; \
    }
    //         type                         floating integer complex fltcmpl  float  double  cfloat  cdouble
    CHECK_TYPE(char                         , NO    , YES   , NO    , NO    , NO    , NO    , NO    , NO   )
    CHECK_TYPE(signed char                  , NO    , YES   , NO    , NO    , NO    , NO    , NO    , NO   )
    CHECK_TYPE(unsigned char                , NO    , YES   , NO    , NO    , NO    , NO    , NO    , NO   )
    CHECK_TYPE(signed short                 , NO    , YES   , NO    , NO    , NO    , NO    , NO    , NO   )
    CHECK_TYPE(unsigned short               , NO    , YES   , NO    , NO    , NO    , NO    , NO    , NO   )
    CHECK_TYPE(signed int                   , NO    , YES   , NO    , NO    , NO    , NO    , NO    , NO   )
    CHECK_TYPE(unsigned int                 , NO    , YES   , NO    , NO    , NO    , NO    , NO    , NO   )
    CHECK_TYPE(signed long                  , NO    , YES   , NO    , NO    , NO    , NO    , NO    , NO   )
    CHECK_TYPE(unsigned long                , NO    , YES   , NO    , NO    , NO    , NO    , NO    , NO   )
    CHECK_TYPE(signed long long             , NO    , YES   , NO    , NO    , NO    , NO    , NO    , NO   )
    CHECK_TYPE(unsigned long long           , NO    , YES   , NO    , NO    , NO    , NO    , NO    , NO   )
    CHECK_TYPE(float                        , YES   , NO    , NO    , NO    , YES   , NO    , NO    , NO   )
    CHECK_TYPE(double                       , YES   , NO    , NO    , NO    , NO    , YES   , NO    , NO   )
    CHECK_TYPE(long double                  , YES   , NO    , NO    , NO    , NO    , MAYBE , NO    , NO   )
    CHECK_TYPE(std::complex<float>          , NO    , NO    , YES   , YES   , NO    , NO    , YES   , NO   )
    CHECK_TYPE(std::complex<double>         , NO    , NO    , YES   , YES   , NO    , NO    , NO    , YES  )
    CHECK_TYPE(std::complex<long double>    , NO    , NO    , YES   , YES   , NO    , NO    , NO    , MAYBE)
#undef CHECK_TYPE
}

void self_test(teestream & tee)
{
    std::vector<std::string> exprs;
    get_all_exprs(exprs);

    evaluator<float> pf;
    evaluator<double> pd;
    evaluator<std::complex<float> > pcf;
    evaluator<std::complex<double> > pcd;

    tee << "Self-Checks\tfloat\tdouble\tcfoat\tcdouble" << std::endl;
    for(std::size_t i = 0; i < exprs.size(); i++)
    {
        tee << exprs[i] << "\t";

        if(!pf.parse(exprs[i]))  std::cerr << pf.get_error() << std::endl;
        if(!pd.parse(exprs[i]))  std::cerr << pd.get_error() << std::endl;
        if(!pcf.parse(exprs[i])) std::cerr << pcf.get_error() << std::endl;
        if(!pcd.parse(exprs[i])) std::cerr << pcd.get_error() << std::endl;

        double xd, yd, rd, sumsd = 0, sumid = 0, sumed = 0;
        float xf, yf, rf, sumsf = 0, sumif = 0, sumef = 0;
        std::complex<float> xcf, ycf, rcf, sumscf = 0, sumicf = 0, sumecf = 0;
        std::complex<double> xcd, ycd, rcd, sumscd = 0, sumicd = 0, sumecd = 0;

        std::size_t num_tests = 50;
        double eps_d = 5e-14;
        float eps_f = 5e-7f;

        srand(1);
        for(std::size_t j = 0; j < num_tests; j++)
        {
            xd = rand_uniform(0, 1);
            xf = static_cast<float>(xd);
            yd = rand_uniform(0, 1);
            yf = static_cast<float>(yd);
            xcd = std::complex<double>(xd, yd);
            ycd = std::complex<double>(yd, xd);
            xcf = std::complex<float>(xf, yf);
            ycf = std::complex<float>(yf, xf);

            if(exprs[i] == "acosh(x)")
            {
                xd += 1.0;
                xf += 1.0f;
            }

            pf.set_var("x", xf);
            pf.set_var("y", yf);
            pd.set_var("x", xd);
            pd.set_var("y", yd);
            pcf.set_var("x", xcf);
            pcf.set_var("y", ycf);
            pcd.set_var("x", xcd);
            pcd.set_var("y", ycd);

            pf.calculate(rf);
            pd.calculate(rd);
            pcf.calculate(rcf);
            pcd.calculate(rcd);

            sumsd += rd;
            sumsf += rf;
            sumscd += rcd;
            sumscf += rcf;
        }

        if(!pf.compile_inline())  std::cerr << pf.get_error() << std::endl;
        if(!pd.compile_inline())  std::cerr << pd.get_error() << std::endl;
        if(!pcf.compile_inline()) std::cerr << pcf.get_error() << std::endl;
        if(!pcd.compile_inline()) std::cerr << pcd.get_error() << std::endl;

        srand(1);
        for(std::size_t j = 0; j < num_tests; j++)
        {
            xd = rand_uniform(0, 1);
            xf = static_cast<float>(xd);
            yd = rand_uniform(0, 1);
            yf = static_cast<float>(yd);
            xcd = std::complex<double>(xd, yd);
            ycd = std::complex<double>(yd, xd);
            xcf = std::complex<float>(xf, yf);
            ycf = std::complex<float>(yf, xf);

            if(exprs[i] == "acosh(x)")
            {
                xd += 1.0;
                xf += 1.0f;
            }

            pf.set_var("x", xf);
            pf.set_var("y", yf);
            pd.set_var("x", xd);
            pd.set_var("y", yd);
            pcf.set_var("x", xcf);
            pcf.set_var("y", ycf);
            pcd.set_var("x", xcd);
            pcd.set_var("y", ycd);

            pf.calculate(rf);
            pd.calculate(rd);
            pcf.calculate(rcf);
            pcd.calculate(rcd);

            sumid += rd;
            sumif += rf;
            sumicd += rcd;
            sumicf += rcf;
        }

        if(!pf.compile_extcall())  std::cerr << pf.get_error() << std::endl;
        if(!pd.compile_extcall())  std::cerr << pd.get_error() << std::endl;
        if(!pcf.compile_extcall()) std::cerr << pcf.get_error() << std::endl;
        if(!pcd.compile_extcall()) std::cerr << pcd.get_error() << std::endl;

        srand(1);
        for(std::size_t j = 0; j < num_tests; j++)
        {
            xd = rand_uniform(0, 1);
            xf = static_cast<float>(xd);
            yd = rand_uniform(0, 1);
            yf = static_cast<float>(yd);
            xcd = std::complex<double>(xd, yd);
            ycd = std::complex<double>(yd, xd);
            xcf = std::complex<float>(xf, yf);
            ycf = std::complex<float>(yf, xf);

            if(exprs[i] == "acosh(x)")
            {
                xd += 1.0;
                xf += 1.0f;
            }

            pf.set_var("x", xf);
            pf.set_var("y", yf);
            pd.set_var("x", xd);
            pd.set_var("y", yd);
            pcf.set_var("x", xcf);
            pcf.set_var("y", ycf);
            pcd.set_var("x", xcd);
            pcd.set_var("y", ycd);

            pf.calculate(rf);
            pd.calculate(rd);
            pcf.calculate(rcf);
            pcd.calculate(rcd);

            sumed += rd;
            sumef += rf;
            sumecd += rcd;
            sumecf += rcf;
        }

        if(((std::abs((sumif - sumsf) / ((sumif + sumsf) / 2.0f)) < eps_f) ||
            (std::abs(sumif) < eps_f && std::abs(sumsf) < eps_f)) &&
           ((std::abs((sumef - sumsf) / ((sumef + sumsf) / 2.0f)) < eps_f) ||
            (std::abs(sumef) < eps_f && std::abs(sumsf) < eps_f)))
        {
            tee << "OK\t";
        }
        else
        {
            tee << "FAIL\t";
        }

        if(((std::abs((sumid - sumsd) / ((sumid + sumsd) / 2.0)) < eps_d) ||
            (std::abs(sumid) < eps_d && std::abs(sumsd) < eps_d)) &&
           ((std::abs((sumed - sumsd) / ((sumed + sumsd) / 2.0)) < eps_d) ||
            (std::abs(sumed) < eps_d && std::abs(sumsd) < eps_d)))
        {
            tee << "OK\t";
        }
        else
        {
            tee << "FAIL\t";
        }

        if(((std::abs((sumicf - sumscf) / ((sumicf + sumscf) / 2.0f)) < eps_f) ||
            (std::abs(sumicf) < eps_f && std::abs(sumscf) < eps_f)) &&
           ((std::abs((sumecf - sumscf) / ((sumecf + sumscf) / 2.0f)) < eps_f) ||
            (std::abs(sumecf) < eps_f && std::abs(sumscf) < eps_f)))
        {
            tee << "OK\t";
        }
        else
        {
            tee << "FAIL\t";
        }

        if(((std::abs((sumicd - sumscd) / ((sumicd + sumscd) / 2.0)) < eps_d) ||
            (std::abs(sumicd) < eps_d && std::abs(sumscd) < eps_d)) &&
           ((std::abs((sumecd - sumscd) / ((sumecd + sumscd) / 2.0)) < eps_d) ||
            (std::abs(sumecd) < eps_d && std::abs(sumscd) < eps_d)))
        {
            tee << "OK\t";
        }
        else
        {
            tee << "FAIL\t";
        }

        tee << std::endl;
    }
}

double rand_uniform(double a, double b)
{
    double alpha1 = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
    return a + alpha1 * (b - a);
}

unsigned long mtime()
{
#if defined(_WIN32)

    return GetTickCount();

#elif defined(__MACH__)

    mach_timebase_info_data_t timebase;
    mach_timebase_info(& timebase);
    uint64_t time = mach_absolute_time();
    return static_cast<unsigned long>
            ((time * static_cast<uint64_t>(timebase.numer)) /
            (static_cast<uint64_t>(timebase.denom) * static_cast<uint64_t>(1000000)));

#else

    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, & t);
    return static_cast<unsigned long>(t.tv_sec * 1000) +
           static_cast<unsigned long>(t.tv_nsec / 1000000);

#endif
}

} // namespace

