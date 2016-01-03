#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include "parser.h"

// cl /Gd /Ox /GS- /EHsc /DNDEBUG /Fe: bench.exe /MD benchmark.cpp
// g++ -O3 -march=native -mtune=native -DNDEBUG benchmark.cpp -o bench

#if defined _WIN32
#include <windows.h>
unsigned long mtime()
{
    return GetTickCount();
}
#else
unsigned long mtime()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, & t);
    return (unsigned long)t.tv_sec * 1000 + t.tv_nsec / 1000000;
}
#endif

using namespace std;

int main(int argc, char * argv[])
{
    size_t num_tests = 50000000;
    if(argc > 1)
        num_tests = (size_t)atoi(argv[1]);

    stringstream data, name;
    name << "result_";

#if defined __GNUC__
    data << "Compiler: GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << endl;
    name << "gcc-" << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "_";
#endif
#if defined __clang__
    data << "Compiler: Clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__ << endl;
    name << "clang-" << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__ << "_";
#endif
#if defined _MSC_VER
    data << "Compiler: VS " << _MSC_VER << " (" << _MSC_FULL_VER << ")" << endl;
    name << "vs-" << _MSC_VER << "_";
#endif
#if defined PARSER_JIT_X64
    data << "Arch: x64" << endl;
    name << "x64_";
#endif
#if defined PARSER_JIT_X86
    data << "Arch: x86" << endl;
    name << "x86_";
#endif
#if defined PARSER_JIT_MSVC_ABI
    data << "ABI: MSVC" << endl;
    name << "abi-ms";
#endif
#if defined PARSER_JIT_SYSV_ABI
    data << "ABI: SYSV" << endl;
    name << "abi-sysv";
#endif

    name << ".txt";
    ofstream ofs(name.str().c_str());
    cout << data.str() << "Data Size: " << num_tests << endl;
    ofs << data.str() << "Data Size: " << num_tests << endl;

    cout << "\n================================" << endl;
    ofs << "\n================================" << endl;

    vector<string> exprs;
    exprs.push_back("x+y");
    exprs.push_back("x-y");
    exprs.push_back("x*y");
    exprs.push_back("x^y");
    exprs.push_back("imag(x)");
    exprs.push_back("real(x)");
    exprs.push_back("conj(x)");
    exprs.push_back("arg(x)");
    exprs.push_back("sin(x)");
    exprs.push_back("cos(x)");
    exprs.push_back("tan(x)");
    exprs.push_back("asin(x)");
    exprs.push_back("acos(x)");
    exprs.push_back("atan(x)");
    exprs.push_back("sinh(x)");
    exprs.push_back("cosh(x)");
    exprs.push_back("tanh(x)");
    exprs.push_back("asinh(x)");
    exprs.push_back("acosh(x)");
    exprs.push_back("atanh(x)");
    exprs.push_back("log(x)");
    exprs.push_back("log2(x)");
    exprs.push_back("log10(x)");
    exprs.push_back("abs(x)");
    exprs.push_back("exp(x)");
    exprs.push_back("sqrt(x)");

    parser<float> pf;
    parser<double> pd;
    parser<complex<float> > pcf;
    parser<complex<double> > pcd;

    for(size_t i = 0; i < exprs.size(); i++)
    {
        cout << "Test: " << exprs[i] << endl;
        cout << "float\tdouble\tcfoat\tcdouble" << endl;
        ofs << "Test: " << exprs[i] << endl;
        ofs << "float\tdouble\tcfoat\tcdouble" << endl;

        if(!pf.parse(exprs[i]))  cout << pf.get_error() << endl;
        if(!pd.parse(exprs[i]))  cout << pd.get_error() << endl;
        if(!pcf.parse(exprs[i])) cout << pcf.get_error() << endl;
        if(!pcd.parse(exprs[i])) cout << pcd.get_error() << endl;

        double xd = (double)i / (double)exprs.size(), rd;
        double yd = 1.0 - (double)i / (double)exprs.size();
        complex<float>  xcf((float)xd, (float)xd), ycf((float)yd, (float)yd), rcf;
        complex<double> xcd(xd, xd), ycd(yd, yd), rcd;
        float xf = (float)xd, yf = (float)yd, rf;

        pf.set_var("x", xf);
        pf.set_var("y", yf);
        pd.set_var("x", xd);
        pd.set_var("y", yd);
        pcf.set_var("x", xcf);
        pcf.set_var("y", ycf);
        pcd.set_var("x", xcd);
        pcd.set_var("y", ycd);

        cout << "---" << endl;
        ofs << "---" << endl;

        if(!pf.compile_extcall())  cout << pf.get_error() << endl;
        if(!pd.compile_extcall())  cout << pd.get_error() << endl;
        if(!pcf.compile_extcall()) cout << pcf.get_error() << endl;
        if(!pcd.compile_extcall()) cout << pcd.get_error() << endl;

        for(size_t j = 0; j < 3; j++)
        {
            unsigned long t;
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pf.calculate(rf);
            t = mtime() - t;
            cout << t << "\t";
            ofs << t << "\t";
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pd.calculate(rd);
            t = mtime() - t;
            cout << t << "\t";
            ofs << t << "\t";
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pcf.calculate(rcf);
            t = mtime() - t;
            cout << t << "\t";
            ofs << t << "\t";
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pcd.calculate(rcd);
            t = mtime() - t;
            cout << t << endl;
            ofs << t << endl;
        }

        cout << "---" << endl;
        ofs << "---" << endl;

        if(!pf.compile_inline())  cout << pf.get_error() << endl;
        if(!pd.compile_inline())  cout << pd.get_error() << endl;
        if(!pcf.compile_inline()) cout << pcf.get_error() << endl;
        if(!pcd.compile_inline()) cout << pcd.get_error() << endl;

        for(size_t j = 0; j < 3; j++)
        {
            unsigned long t;
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pf.calculate(rf);
            t = mtime() - t;
            cout << t << "\t";
            ofs << t << "\t";
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pd.calculate(rd);
            t = mtime() - t;
            cout << t << "\t";
            ofs << t << "\t";
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pcf.calculate(rcf);
            t = mtime() - t;
            cout << t << "\t";
            ofs << t << "\t";
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pcd.calculate(rcd);
            t = mtime() - t;
            cout << t << endl;
            ofs << t << endl;
        }

        cout << "\n================================" << endl;
        ofs << "\n================================" << endl;
    }

#if defined _WIN32
    system("pause");
#endif
    return 0;
}

