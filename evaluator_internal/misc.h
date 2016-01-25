#ifndef MISC_H
#define MISC_H

#include <vector>
#include <complex>
#include <limits>
#include <cstring>
#include "../evaluator.h"
#include "../evaluator_operations.h"

// Return incorrect big number (uninitialized variable)
template<typename T>
template<typename U>
T evaluator<T>::incorrect_number(const std::complex<U> &) const
{
    U num = std::numeric_limits<U>::max();
    return static_cast<T>(std::complex<U>(num, num));
}

template<typename T>
template<typename U>
T evaluator<T>::incorrect_number(const U &) const
{
    return static_cast<T>(std::numeric_limits<U>::max());
}

// Check whether a number is incorrect (uninitialized variable)
template<typename T>
template<typename U>
bool evaluator<T>::is_incorrect(const std::complex<U> & val) const
{
    static U num = std::numeric_limits<U>::max() * (static_cast<U>(1.0) - static_cast<U>(1e-3));
    return val.real() >= num && val.imag() >= num;
}

template<typename T>
template<typename U>
bool evaluator<T>::is_incorrect(const U & val) const
{
    static U num = std::numeric_limits<U>::max() * (static_cast<U>(1.0) - static_cast<U>(1e-3));
    return val >= num;
}

// Primary initialization
template<typename T>
void evaluator<T>::init()
{
    using namespace std;
    using namespace evaluator_internal;
    status = false;
#if !defined(EVALUATOR_JIT_DISABLE)
    is_compiled = false;
    jit_code = NULL;
    jit_code_size = 0;
    jit_stack = NULL;
    jit_stack_size = 0;
    jit_func = NULL;
#endif
    init_functions(functions);
    init_operators(operators);
    init_variables(variables);

    transition_table.resize(29);
    transition_table[ 0].set_values("func var const sign (",  1, false, false, false, true );
    transition_table[ 1].set_values("func var const sign (",  2, false, false, false, true );
    transition_table[ 2].set_values("sign",                   4, false, false, false, false);
    transition_table[ 3].set_values("func var const (",       6, false, false, false, true );
    transition_table[ 4].set_values("sign",                   5, true,  false, false, true );
    transition_table[ 5].set_values("func var const (",       7, false, false, false, true );
    transition_table[ 6].set_values("func var const (",       7, false, false, false, true );
    transition_table[ 7].set_values("var",                   11, false, false, false, false);
    transition_table[ 8].set_values("const",                 13, false, false, false, false);
    transition_table[ 9].set_values("(",                     15, false, false, false, false);
    transition_table[10].set_values("func",                  19, false, false, false, true );
    transition_table[11].set_values("var",                   12, true,  false, false, true );
    transition_table[12].set_values("oper eps",              24, false, false, false, true );
    transition_table[13].set_values("const",                 14, true,  false, false, true );
    transition_table[14].set_values("oper eps",              24, false, false, false, true );
    transition_table[15].set_values("(",                     16, true,  false, false, true );
    transition_table[16].set_values("func var const sign (",  2, false, true,  false, true );
    transition_table[17].set_values(")",                     18, true,  false, false, true );
    transition_table[18].set_values("oper eps",              24, false, false, false, true );
    transition_table[19].set_values("func",                  20, true,  false, false, true );
    transition_table[20].set_values("(",                     21, true,  false, false, true );
    transition_table[21].set_values("func var const sign (",  2, false, true,  false, true );
    transition_table[22].set_values(")",                     23, true,  false, false, true );
    transition_table[23].set_values("oper eps",              24, false, false, false, true );
    transition_table[24].set_values("oper",                  27, false, false, false, false);
    transition_table[25].set_values("eps",                   26, false, false, false, true );
    transition_table[26].set_values("eps",                   -1, false, false, true,  true );
    transition_table[27].set_values("oper",                  28, true,  false, false, true );
    transition_table[28].set_values("func var const (",       7, false, false, false, true );
}

// Copying from another evaluator
template<typename T>
void evaluator<T>::copy_from_other(const evaluator & other)
{
    expression = other.expression;
    functions = other.functions;
    variables = other.variables;
    operators = other.operators;
    status = other.status;
    error_string = other.error_string;
    transition_table = other.transition_table;
#if !defined(EVALUATOR_JIT_DISABLE)
    is_compiled = false;
    jit_code = NULL;
    jit_code_size = 0;
    jit_stack = NULL;
    jit_stack_size = 0;
    jit_func = NULL;
#endif
}

// Reset all variables
template<typename T>
void evaluator<T>::reset_vars()
{
    using namespace std;
    using namespace evaluator_internal;
    variables.clear();
    init_variables(variables);
    if(is_parsed())
        for(typename vector<evaluator_object<T> >::iterator
            it = expression.begin(); it != expression.end(); ++it)
            variables[it->str()].value() = incorrect_number(T());
}

// Constructors and destructor
template<typename T>
evaluator<T>::evaluator(const evaluator & other)
{
    copy_from_other_parser(other);
}

template<typename T>
evaluator<T>::evaluator()
{
    init();
}

template<typename T>
evaluator<T>::evaluator(const std::string & str)
{
    init();
    parse(str);
}

template<typename T>
evaluator<T>::~evaluator()
{
#if !defined(EVALUATOR_JIT_DISABLE)
    if(jit_code && jit_code_size)
    {
        evaluator_internal_jit::exec_dealloc(jit_code, jit_code_size);
    }
    if(jit_stack && jit_stack_size)
    {
        delete [] jit_stack;
    }
#endif
}

// Copying from another evaluator
template<typename T>
const evaluator<T> & evaluator<T>::operator = (const evaluator & other)
{
    if(this != &other)
        copy_from_other(other);
    return * this;
}

#endif // MISC_H

