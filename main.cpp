#include <iostream>
#include <cstdlib>
#include "parser.h"

using namespace std;

int main()
{
    parser<double> p;
    //parser<complex<double> > p;
    //parser<int> p;
    p.parse("2*x+y");
    p.simplify();
    p.set_const("x", 2);
    p.set_const("y", 2);
    p.debug_print();
    double result = 42;
    //complex<double> result;
    //int result;
    if(!p.calculate(result))
        cout << p.get_error() << endl;
    else
        cout << result << endl;
#if defined _WIN32
    system("pause");
#endif
    return 0;
}

