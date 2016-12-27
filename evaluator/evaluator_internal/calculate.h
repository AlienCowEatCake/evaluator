#if !defined(EVALUATOR_CALCULATE_H)
#define EVALUATOR_CALCULATE_H

#include <stack>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include "../evaluator.h"

// Calculate current expression and write result to 'result'
template<typename T>
bool evaluator<T>::calculate(T & result)
{
    using namespace evaluator_internal;

    if(!is_parsed())
    {
        m_error_string = "Not parsed!";
        return false;
    }

#if !defined(EVALUATOR_JIT_DISABLE)
    if(m_is_compiled)
    {
        m_jit_func();
        result = m_jit_stack[0];
        return true;
    }
#endif

    std::stack<T> st;

    for(typename std::vector<evaluator_object<T> >::const_iterator
        it = m_expression.begin(), it_end = m_expression.end(); it != it_end; ++it)
    {
        if(it->is_constant())
        {
            st.push(it->eval());
        }
        else if(it->is_variable())
        {
            const T val = it->eval();
            if(!is_incorrect(val))
            {
                st.push(val);
            }
            else
            {
                std::stringstream sst;
                sst << "Constant `" << it->str() << "` must be defined!";
                m_error_string = sst.str();
                return false;
            }
        }
        else if(it->is_operator())
        {
            const T arg2 = st.top();
            st.pop();
            const T arg1 = st.top();
            st.pop();
            const T val = it->eval(arg1, arg2);
            st.push(val);
        }
        else if(it->is_function())
        {
            const T arg = st.top();
            st.pop();
            const T val = it->eval(arg);
            st.push(val);
        }
    }

    if(st.size() != 1)
    {
        std::stringstream sst;
        sst << "Stack size equal " << st.size();
        m_error_string = sst.str();
        return false;
    }
    result = st.top();
    return true;
}

#endif // EVALUATOR_CALCULATE_H

