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

int main()
{
    /*
    map<int, parser_internal::var_container<double> > q;
    q[10] = 1;
    cout << q[10].pointer() << endl;
    q[11] = 1;
    q[12] = 1;
    q[13] = 1;
    q[10].value() = 1;
    cout << q[10].pointer() << endl;
    */

    typedef double T;

    T result = 42;
    parser<T> p;
    //p.parse("x+y");
    p.parse("2-(2+(5*3+2))");
    //p.parse("2^3");
    //p.simplify();
    //p.parse("exp(- (0.5 - x) * (0.5 - x) - (0.5 - z) * (0.5 - z))");
    //p.parse("exp(log(cos(acos(0.8))))");
    //p.parse("atanh(-0.854)");
    p.set_const("x", 2);
    p.set_const("y", 2);
    p.set_const("z", 2);

    if(!p.calculate(result))
        cout << p.get_error() << endl;
    else
        cout << result << endl;

    size_t exp_num = 100000000;
    T sum = 0;
    long t = mtime();
    for(size_t i = 0; i < exp_num; i++)
    {
        T result;
        p.calculate(result);
        sum += result;
    }
    t = mtime() - t;
    cout << "Stack: " << t << endl;

    if(!p.compile())
        cout << p.get_error() << endl;
    //p.set_const("x", 2);
    //p.set_const("y", 2);
//    p.debug_print();
    if(!p.calculate(result))
        cout << p.get_error() << endl;
    else
        cout << result << endl;
    if(!p.calculate(result))
        cout << p.get_error() << endl;
    else
        cout << result << endl;

    t = mtime();
    for(size_t i = 0; i < exp_num; i++)
    {
        T result;
        p.calculate(result);
        sum += result;
    }
    t = mtime() - t;
    cout << "JIT: " << t << endl;

    t = mtime();
    for(size_t i = 0; i < exp_num; i++)
    {
        T result;
        T x = 2, z = 2;
        result = exp(- (0.5 - x) * (0.5 - x) - (0.5 - z) * (0.5 - z));
        sum += result;
    }
    t = mtime() - t;
    cout << "Native: " << t << endl;

    cout << sum << endl;

#if defined _WIN32
    system("pause");
#endif
    return 0;
}

