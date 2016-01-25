#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include "evaluator.h"

using namespace std;

unsigned long mtime();
double rand_uniform(double a, double b);
void self_test(ofstream & ofs);
void benchmark1(size_t num_tests, ofstream & ofs);

int main(int argc, char * argv[])
{
    size_t num_tests = 50000000;
    if(argc > 1)
        num_tests = (size_t)atoi(argv[1]);

    stringstream data, name;
    name << "result_";

#if defined(__GNUC__) && !defined(__clang__)
    data << "Compiler: GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << endl;
    name << "gcc-" << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "_";
#endif
#if defined(__clang__)
    data << "Compiler: Clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__ << endl;
    name << "clang-" << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__ << "_";
#endif
#if defined(_MSC_VER)
    data << "Compiler: VS " << _MSC_VER << " (" << _MSC_FULL_VER << ")" << endl;
    name << "vs-" << _MSC_VER << "_";
#endif
#if defined(EVALUATOR_JIT_X64)
    data << "Arch: x64" << endl;
    name << "x64_";
#endif
#if defined(EVALUATOR_JIT_X86)
    data << "Arch: x86" << endl;
    name << "x86_";
#endif
#if defined(EVALUATOR_JIT_MSVC_ABI)
    data << "ABI: MSVC" << endl;
    name << "abi-msvc";
#endif
#if defined(EVALUATOR_JIT_SYSV_ABI)
    data << "ABI: SYSV" << endl;
    name << "abi-sysv";
#endif
#if defined(EVALUATOR_JIT_MINGW_ABI)
    data << "ABI: MinGW" << endl;
    name << "abi-mingw";
#endif

    name << ".txt";
    ofstream ofs(name.str().c_str());
    cout << data.str() << "Data Size: " << num_tests << endl;
    ofs  << data.str() << "Data Size: " << num_tests << endl;

    cout << "\n================================" << endl;
    ofs  << "\n================================" << endl;

    self_test(ofs);

    cout << "\n================================" << endl;
    ofs  << "\n================================" << endl;

    benchmark1(num_tests, ofs);

#if defined(_WIN32)
    system("pause");
#endif
    return 0;
}

void get_all_exprs(vector<string> & exprs)
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

void benchmark1(size_t num_tests, ofstream & ofs)
{
    vector<string> exprs;
    get_all_exprs(exprs);

    evaluator<float> pf;
    evaluator<double> pd;
    evaluator<complex<float> > pcf;
    evaluator<complex<double> > pcd;

    for(size_t i = 0; i < exprs.size(); i++)
    {
        cout << "Test: " << exprs[i] << endl;
        cout << "float\tdouble\tcfoat\tcdouble" << endl;
        ofs  << "Test: " << exprs[i] << endl;
        ofs  << "float\tdouble\tcfoat\tcdouble" << endl;

        if(!pf.parse(exprs[i]))  cout << pf.get_error() << endl;
        if(!pd.parse(exprs[i]))  cout << pd.get_error() << endl;
        if(!pcf.parse(exprs[i])) cout << pcf.get_error() << endl;
        if(!pcd.parse(exprs[i])) cout << pcd.get_error() << endl;

        double xd = rand_uniform(0, 1);
        float xf = (float)xd;
        double yd = rand_uniform(0, 1);
        float yf = (float)yd;
        complex<double> xcd = complex<double>(xd, yd);
        complex<double> ycd = complex<double>(yd, xd);
        complex<float> xcf = complex<float>(xf, yf);
        complex<float> ycf = complex<float>(yf, xf);
        double rd;
        float rf;
        complex<double> rcd;
        complex<float> rcf;

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

        cout << "--- Extcall: ---" << endl;
        ofs  << "--- Extcall: ---" << endl;

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
            ofs  << t << "\t";
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pd.calculate(rd);
            t = mtime() - t;
            cout << t << "\t";
            ofs  << t << "\t";
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pcf.calculate(rcf);
            t = mtime() - t;
            cout << t << "\t";
            ofs  << t << "\t";
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pcd.calculate(rcd);
            t = mtime() - t;
            cout << t << endl;
            ofs  << t << endl;
        }

        cout << "--- Inline: ---" << endl;
        ofs  << "--- Inline: ---" << endl;

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
            ofs  << t << "\t";
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pd.calculate(rd);
            t = mtime() - t;
            cout << t << "\t";
            ofs  << t << "\t";
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pcf.calculate(rcf);
            t = mtime() - t;
            cout << t << "\t";
            ofs  << t << "\t";
            t = mtime();
            for(size_t k = 0; k < num_tests; k++)
                pcd.calculate(rcd);
            t = mtime() - t;
            cout << t << endl;
            ofs  << t << endl;
        }

        cout << "\n--------------------------------" << endl;
        ofs  << "\n--------------------------------" << endl;
    }
}

void self_test(ofstream & ofs)
{
    vector<string> exprs;
    get_all_exprs(exprs);

    evaluator<float> pf;
    evaluator<double> pd;
    evaluator<complex<float> > pcf;
    evaluator<complex<double> > pcd;

    cout << "Self-Checks\tfloat\tdouble\tcfoat\tcdouble" << endl;
    ofs  << "Self-Checks\tfloat\tdouble\tcfoat\tcdouble" << endl;
    for(size_t i = 0; i < exprs.size(); i++)
    {
        cout << exprs[i] << "\t";
        ofs  << exprs[i] << "\t";

        if(!pf.parse(exprs[i]))  cout << pf.get_error() << endl;
        if(!pd.parse(exprs[i]))  cout << pd.get_error() << endl;
        if(!pcf.parse(exprs[i])) cout << pcf.get_error() << endl;
        if(!pcd.parse(exprs[i])) cout << pcd.get_error() << endl;

        double xd, yd, rd, sumsd = 0, sumid = 0, sumed = 0;
        float xf, yf, rf, sumsf = 0, sumif = 0, sumef = 0;
        complex<float> xcf, ycf, rcf, sumscf = 0, sumicf = 0, sumecf = 0;
        complex<double> xcd, ycd, rcd, sumscd = 0, sumicd = 0, sumecd = 0;

        size_t num_tests = 50;
        double eps_d = 5e-14;
        double eps_f = 5e-7;

        srand(1);
        for(size_t j = 0; j < num_tests; j++)
        {
            xd = rand_uniform(0, 1);
            xf = (float)xd;
            yd = rand_uniform(0, 1);
            yf = (float)yd;
            xcd = complex<double>(xd, yd);
            ycd = complex<double>(yd, xd);
            xcf = complex<float>(xf, yf);
            ycf = complex<float>(yf, xf);

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

        if(!pf.compile_inline())  cout << pf.get_error() << endl;
        if(!pd.compile_inline())  cout << pd.get_error() << endl;
        if(!pcf.compile_inline()) cout << pcf.get_error() << endl;
        if(!pcd.compile_inline()) cout << pcd.get_error() << endl;

        srand(1);
        for(size_t j = 0; j < num_tests; j++)
        {
            xd = rand_uniform(0, 1);
            xf = (float)xd;
            yd = rand_uniform(0, 1);
            yf = (float)yd;
            xcd = complex<double>(xd, yd);
            ycd = complex<double>(yd, xd);
            xcf = complex<float>(xf, yf);
            ycf = complex<float>(yf, xf);

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

        if(!pf.compile_extcall())  cout << pf.get_error() << endl;
        if(!pd.compile_extcall())  cout << pd.get_error() << endl;
        if(!pcf.compile_extcall()) cout << pcf.get_error() << endl;
        if(!pcd.compile_extcall()) cout << pcd.get_error() << endl;

        srand(1);
        for(size_t j = 0; j < num_tests; j++)
        {
            xd = rand_uniform(0, 1);
            xf = (float)xd;
            yd = rand_uniform(0, 1);
            yf = (float)yd;
            xcd = complex<double>(xd, yd);
            ycd = complex<double>(yd, xd);
            xcf = complex<float>(xf, yf);
            ycf = complex<float>(yf, xf);

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

        if(((fabs((sumif - sumsf) / ((sumif + sumsf) / 2.0f)) < eps_f) ||
            (fabs(sumif) < eps_f && fabs(sumsf) < eps_f)) &&
           ((fabs((sumef - sumsf) / ((sumef + sumsf) / 2.0f)) < eps_f) ||
            (fabs(sumef) < eps_f && fabs(sumsf) < eps_f)))
        {
            cout << "OK\t";
            ofs  << "OK\t";
        }
        else
        {
            cout << "FAIL\t";
            ofs  << "FAIL\t";
        }

        if(((fabs((sumid - sumsd) / ((sumid + sumsd) / 2.0)) < eps_d) ||
            (fabs(sumid) < eps_d && fabs(sumsd) < eps_d)) &&
           ((fabs((sumed - sumsd) / ((sumed + sumsd) / 2.0)) < eps_d) ||
            (fabs(sumed) < eps_d && fabs(sumsd) < eps_d)))
        {
            cout << "OK\t";
            ofs  << "OK\t";
        }
        else
        {
            cout << "FAIL\t";
            ofs  << "FAIL\t";
        }

        if(((abs((sumicf - sumscf) / ((sumicf + sumscf) / 2.0f)) < eps_f) ||
            (abs(sumicf) < eps_f && abs(sumscf) < eps_f)) &&
           ((abs((sumecf - sumscf) / ((sumecf + sumscf) / 2.0f)) < eps_f) ||
            (abs(sumecf) < eps_f && abs(sumscf) < eps_f)))
        {
            cout << "OK\t";
            ofs  << "OK\t";
        }
        else
        {
            cout << "FAIL\t";
            ofs  << "FAIL\t";
        }

        if(((abs((sumicd - sumscd) / ((sumicd + sumscd) / 2.0)) < eps_d) ||
            (abs(sumicd) < eps_d && abs(sumscd) < eps_d)) &&
           ((abs((sumecd - sumscd) / ((sumecd + sumscd) / 2.0)) < eps_d) ||
            (abs(sumecd) < eps_d && abs(sumscd) < eps_d)))
        {
            cout << "OK\t";
            ofs  << "OK\t";
        }
        else
        {
            cout << "FAIL\t";
            ofs  << "FAIL\t";
        }

        cout << endl;
        ofs  << endl;
    }
}

double rand_uniform(double a, double b)
{
    double alpha1 = (double)rand() / (double)RAND_MAX;
    return a + alpha1 * (b - a);
}

#if defined(_WIN32)
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
