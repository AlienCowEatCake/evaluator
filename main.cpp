#include <iostream>
#include <cstdlib>
#include "evaluator.h"

using namespace std;

int main()
{
    evaluator<double> p;
    if(!p.parse("exp(-(0.5-x)*(0.5-x)-(0.5-z)*(0.5-z))"))
        cerr << p.get_error() << endl;
    else if(!p.simplify()) // simplify is optional step
        cerr << p.get_error() << endl;
    else
    {
        if(!p.compile()) // compile is optional step
            cerr << p.get_error() << endl;
        p.set_var("x", 0.4);
        p.set_var("z", 0.8);
        double result;
        if(!p.calculate(result))
            cerr << p.get_error() << endl;
        else
            cout << result << endl;
    }

#if defined(_WIN32)
    system("pause");
#endif
    return 0;
}

