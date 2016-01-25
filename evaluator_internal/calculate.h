#ifndef CALCULATE_H
#define CALCULATE_H

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
    using namespace std;
    using namespace evaluator_internal;

    if(!is_parsed())
    {
        error_string = "Not parsed!";
        return false;
    }

#if !defined(EVALUATOR_JIT_DISABLE)
    if(is_compiled)
    {
        jit_func();
        result = jit_stack[0];
        return true;
    }
#endif

    stack<T> st;

    for(typename vector<evaluator_object<T> >::const_iterator
        it = expression.begin(); it != expression.end(); ++it)
    {
        if(it->is_constant())
        {
            st.push(it->eval());
        }
        else if(it->is_variable())
        {
            T val = it->eval();
            if(!is_incorrect(val))
            {
                st.push(val);
            }
            else
            {
                stringstream sst;
                sst << "Constant `" << it->str() << "` must be defined!";
                error_string = sst.str();
                return false;
            }
        }
        else if(it->is_operator())
        {
            T arg2 = st.top();
            st.pop();
            T arg1 = st.top();
            st.pop();
            T val = it->eval(arg1, arg2);
            st.push(val);
        }
        else if(it->is_function())
        {
            T arg = st.top();
            st.pop();
            T val = it->eval(arg);
            st.push(val);
        }
    }

    if(st.size() != 1)
    {
        stringstream sst;
        sst << "Stack size equal " << st.size();
        error_string = sst.str();
        return false;
    }
    result = st.top();
    return true;
}

#endif // CALCULATE_H

