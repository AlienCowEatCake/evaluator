#include <iostream>
#include <cstdlib>
#include <ctime>
#include "parser.h"

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

#define USE_COMPLEX

int main()
{
#if defined USE_COMPLEX
    typedef complex<double> T;
    T x = T(0.2, 0.3);
    T y = T(-0.4, -0.5);
    T z = T(0.6, 0.7);
#else
    typedef double T;
    T x = 0.2;
    T y = -0.4;
    T z = 0.6;
#endif

    //T I = T(0, 1);
    //cout << (T)1 - I * x << " " << I * x + (T)1 << endl;
    //cout << log((T)1 - I * x) << " " << log(I * x + (T)1) << endl;

    string expr;
    expr = "atanh(z)";
    //expr = "2+2^3-3";
    //expr = "(-1)^2";//"2*atan((sqrt((-1)^2)) / (-1))";//"arg(-i)";
    //expr = "exp(- (0.5 - x) * (0.5 - x) - (0.5 - z) * (0.5 - z))";
    //expr = "atan(x)";
    //expr = "2^3";
    //expr = "(x*y)+(x/y)-(x*x/y)";
    //expr = "x+y";
    //expr = "2-(2+(5*3+2))";
    //expr = "2^3";
    //expr = "exp(- (0.5 - x) * (0.5 - x) - (0.5 - z) * (0.5 - z))";
    //expr = "exp(log(cos(acos(0.8))))";
    //expr = "atanh(-0.854)";

    T result = 42;
    parser<T> p;
    if(!p.parse(expr))
    {
        cout << p.get_error() << endl;
#if defined _WIN32
        system("pause");
#endif
        return 1;
    }
    //p.simplify();
    //p.debug_print();

    p.set_var("x", x);
    p.set_var("y", y);
    p.set_var("z", z);

    if(!p.calculate(result))
        cout << p.get_error() << endl;
    else
        cout << "Stack result:   " << result << endl;

    size_t exp_num = 100000;
    T sum = 0;
    unsigned long t_st = mtime();
    for(size_t i = 0; i < exp_num; i++)
    {
        T result;
        p.calculate(result);
        sum += result;
    }
    t_st = mtime() - t_st;

    if(!p.compile())
        cout << p.get_error() << endl;

    if(!p.calculate(result))
        cout << p.get_error() << endl;
    else
        cout << "JITe result #1: " << result << endl;
    if(!p.calculate(result))
        cout << p.get_error() << endl;
    else
        cout << "JITe result #2: " << result << endl;

    unsigned long t_jit = mtime();
    for(size_t i = 0; i < exp_num; i++)
    {
        T result;
        p.calculate(result);
        sum += result;
    }
    t_jit = mtime() - t_jit;

    if(!p.compile_inline())
        cout << p.get_error() << endl;

    if(!p.calculate(result))
        cout << p.get_error() << endl;
    else
        cout << "JITi result #1: " << result << endl;
    if(!p.calculate(result))
        cout << p.get_error() << endl;
    else
        cout << "JITi result #2: " << result << endl;

    unsigned long t_jit_i = mtime();
    for(size_t i = 0; i < exp_num; i++)
    {
        T result;
        p.calculate(result);
        sum += result;
    }
    t_jit_i = mtime() - t_jit_i;

    unsigned long t_n = mtime();
    for(size_t i = 0; i < exp_num; i++)
    {
        T result;
        result = exp(- (0.5 - x) * (0.5 - x) - (0.5 - z) * (0.5 - z));
        //result = asin(x);
        sum += result;
    }
    t_n = mtime() - t_n;

    cout << "Stack: " << t_st << endl;
    cout << "JITe: " << t_jit << endl;
    cout << "JITi: " << t_jit_i << endl;
    cout << "Native: " << t_n << endl;

    cout << sum << endl;

#if defined _WIN32
    system("pause");
#endif
    return 0;
}

