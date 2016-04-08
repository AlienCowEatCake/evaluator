#ifndef EVALUATOR_MISC_H
#define EVALUATOR_MISC_H

#include <vector>
#include <complex>
#include <limits>
#include <cstring>
#include <iostream>
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
    m_status = false;
#if !defined(EVALUATOR_JIT_DISABLE)
    m_is_compiled = false;
    m_jit_code = NULL;
    m_jit_code_size = 0;
    m_jit_stack = NULL;
    m_jit_stack_size = 0;
    m_jit_func = NULL;
#endif
    init_functions(m_functions);
    init_operators(m_operators);
    init_constants(m_constants);

    m_transition_table.resize(29);
    m_transition_table[ 0].set_values("func var const sign (",  1, false, false, false, true );
    m_transition_table[ 1].set_values("func var const sign (",  2, false, false, false, true );
    m_transition_table[ 2].set_values("sign",                   4, false, false, false, false);
    m_transition_table[ 3].set_values("func var const (",       6, false, false, false, true );
    m_transition_table[ 4].set_values("sign",                   5, true,  false, false, true );
    m_transition_table[ 5].set_values("func var const (",       7, false, false, false, true );
    m_transition_table[ 6].set_values("func var const (",       7, false, false, false, true );
    m_transition_table[ 7].set_values("var",                   11, false, false, false, false);
    m_transition_table[ 8].set_values("const",                 13, false, false, false, false);
    m_transition_table[ 9].set_values("(",                     15, false, false, false, false);
    m_transition_table[10].set_values("func",                  19, false, false, false, true );
    m_transition_table[11].set_values("var",                   12, true,  false, false, true );
    m_transition_table[12].set_values("oper eps",              24, false, false, false, true );
    m_transition_table[13].set_values("const",                 14, true,  false, false, true );
    m_transition_table[14].set_values("oper eps",              24, false, false, false, true );
    m_transition_table[15].set_values("(",                     16, true,  false, false, true );
    m_transition_table[16].set_values("func var const sign (",  2, false, true,  false, true );
    m_transition_table[17].set_values(")",                     18, true,  false, false, true );
    m_transition_table[18].set_values("oper eps",              24, false, false, false, true );
    m_transition_table[19].set_values("func",                  20, true,  false, false, true );
    m_transition_table[20].set_values("(",                     21, true,  false, false, true );
    m_transition_table[21].set_values("func var const sign (",  2, false, true,  false, true );
    m_transition_table[22].set_values(")",                     23, true,  false, false, true );
    m_transition_table[23].set_values("oper eps",              24, false, false, false, true );
    m_transition_table[24].set_values("oper",                  27, false, false, false, false);
    m_transition_table[25].set_values("eps",                   26, false, false, false, true );
    m_transition_table[26].set_values("eps",                   -1, false, false, true,  true );
    m_transition_table[27].set_values("oper",                  28, true,  false, false, true );
    m_transition_table[28].set_values("func var const (",       7, false, false, false, true );
}

// Copying from another evaluator
template<typename T>
void evaluator<T>::copy_from_other(const evaluator & other)
{
    m_expression = other.m_expression;
    m_functions = other.m_functions;
    m_variables = other.m_variables;
    m_constants = other.m_constants;
    m_operators = other.m_operators;
    m_status = other.m_status;
    m_error_string = other.m_error_string;
    m_transition_table = other.m_transition_table;
#if !defined(EVALUATOR_JIT_DISABLE)
    m_is_compiled = false;
    m_jit_code = NULL;
    m_jit_code_size = 0;
    m_jit_stack = NULL;
    m_jit_stack_size = 0;
    m_jit_func = NULL;
#endif
}

// Reset all variables
template<typename T>
void evaluator<T>::reset_vars()
{
    using namespace std;
    using namespace evaluator_internal;
    m_variables.clear();
    if(is_parsed())
        for(typename vector<evaluator_object<T> >::iterator
            it = m_expression.begin(), it_end = m_expression.end(); it != it_end; ++it)
            if(it->is_variable())
            {
                m_variables[it->str()].value() = incorrect_number(T());
                *it = evaluator_object<T>(it->str(), m_variables[it->str()].pointer());
            }
}

// Constructors and destructor
template<typename T>
evaluator<T>::evaluator(const evaluator & other)
{
    copy_from_other(other);
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
    if(m_jit_code && m_jit_code_size)
    {
        evaluator_internal_jit::exec_dealloc(m_jit_code, m_jit_code_size);
    }
    if(m_jit_stack && m_jit_stack_size)
    {
        delete [] m_jit_stack;
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

// Print expression
template<typename T>
void evaluator<T>::debug_print() const
{
    using namespace std;
    using namespace evaluator_internal;
    for(typename vector<evaluator_object<T> >::const_iterator
        it = m_expression.begin(), it_end = m_expression.end(); it != it_end; ++it)
    {
        cout << it->str();
        if(it->is_variable())
            cout << "->" << it->eval() << ' ';
        else
            cout << ' ';
    }
    cout << endl;
}

#endif // EVALUATOR_MISC_H

