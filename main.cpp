#include <iostream>
#include <cstdlib>
#include "evaluator/evaluator.h"
#include "evaluator/evaluator_xyz.h"

int main()
{
    evaluator<double> p;
    if(!p.parse("exp(-(0.5-x)*(0.5-x)-(0.5-z)*(0.5-z))"))
        std::cerr << p.get_error() << std::endl;
    else if(!p.simplify()) // simplify is optional step
        std::cerr << p.get_error() << std::endl;
    else
    {
        if(!p.compile()) // compile is optional step
            std::cerr << p.get_error() << std::endl;
        p.set_var("x", 0.4);
        p.set_var("z", 0.8);
        double result;
        if(!p.calculate(result))
            std::cerr << p.get_error() << std::endl;
        else
            std::cout << result << std::endl;
    }

    evaluator_xyz<double> e;
    if(!e.parse("exp(-(0.5-x)*(0.5-x)-(0.5-z)*(0.5-z))"))
        std::cerr << e.get_error() << std::endl;
    else if(!e.simplify()) // simplify is optional step
        std::cerr << e.get_error() << std::endl;
    else
    {
        if(!e.compile()) // compile is optional step
            std::cerr << e.get_error() << std::endl;
        e.set_x(0.4);
        e.set_z(0.8);
        double result;
        if(!e.calculate(result))
            std::cerr << e.get_error() << std::endl;
        else
            std::cout << result << std::endl;
    }

#if defined(_WIN32)
    system("pause");
#endif
    return 0;
}

