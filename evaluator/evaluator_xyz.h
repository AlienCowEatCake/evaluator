#if !defined(EVALUATOR_XYZ_H)
#define EVALUATOR_XYZ_H

/*
// Usage example:
//
// #include "evaluator/evaluator_xyz.h"
// ...
// evaluator_xyz<double> e;
// if(!e.parse("exp(-(0.5-x)*(0.5-x)-(0.5-z)*(0.5-z))"))
//     std::cerr << e.get_error() << std::endl;
// else if(!e.simplify()) // simplify is optional step
//     std::cerr << e.get_error() << std::endl;
// else
// {
//     if(!e.compile()) // compile is optional step
//         std::cerr << e.get_error() << std::endl;
//     e.set_x(0.4);
//     e.set_z(0.8);
//     double result;
//     if(!e.calculate(result))
//         std::cerr << e.get_error() << std::endl;
//     else
//         std::cout << result << std::endl;
// }
*/

#include "evaluator.h"

// Reference example: Evaluator(x,y,z), takes O(1) time for setting x, y or z
template<typename T> class evaluator_xyz : public evaluator<T>
{
public:

    // Constructors
    evaluator_xyz() : evaluator<T>()
    {
        update_cache();
    }

    evaluator_xyz(const std::string & str) : evaluator<T>(str)
    {
        update_cache();
    }

    evaluator_xyz(const evaluator_xyz & other) : evaluator<T>(other)
    {
        update_cache();
    }

    // Copying from another evaluator
    const evaluator_xyz & operator = (const evaluator_xyz & other)
    {
        evaluator<T>::copy_from_other(other);
        update_cache();
        return * this;
    }

    // Set new value 'x' for variable with name 'x'
    void set_x(const T & x)
    {
        * m_x = x;
    }

    // Set new value 'y' for variable with name 'y'
    void set_y(const T & y)
    {
        * m_y = y;
    }

    // Set new value 'z' for variable with name 'z'
    void set_z(const T & z)
    {
        * m_z = z;
    }

private:

    // Cached pointers for x, y and z
    T * m_x, * m_y, * m_z;

    // Update cached pointers
    void update_cache()
    {
        m_x = evaluator<T>::m_variables["x"].pointer();
        m_y = evaluator<T>::m_variables["y"].pointer();
        m_z = evaluator<T>::m_variables["z"].pointer();
    }
};

#endif // EVALUATOR_XYZ_H

