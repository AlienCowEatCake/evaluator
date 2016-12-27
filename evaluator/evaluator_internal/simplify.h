#if !defined(EVALUATOR_SIMPLIFY_H)
#define EVALUATOR_SIMPLIFY_H

#include <deque>
#include <sstream>
#include <vector>
#include <string>
#include "../evaluator.h"

// Simplify current expression
template<typename T>
bool evaluator<T>::simplify()
{
    using namespace evaluator_internal;

    if(!is_parsed())
    {
        m_error_string = "Not parsed!";
        return false;
    }

    bool was_changed;
    do
    {
        std::deque<evaluator_object<T> > dq;
        was_changed = false;

        for(typename std::vector<evaluator_object<T> >::iterator
            it = m_expression.begin(), it_end = m_expression.end(); it != it_end; ++it)
        {
            if(it->is_operator())
            {
                const evaluator_object<T> arg2 = dq.back();
                if(arg2.is_constant())
                {
                    dq.pop_back();
                    const evaluator_object<T> arg1 = dq.back();
                    if(arg1.is_constant())
                    {
                        dq.pop_back();
                        const T varg1 = arg1.eval();
                        const T varg2 = arg2.eval();
                        const T val = it->eval(varg1, varg2);
                        std::stringstream sst;
                        sst.precision(17);
                        sst.setf(std::ios::scientific);
                        sst << val;
                        const std::string sst_st = sst.str();
                        dq.push_back(evaluator_object<T>(sst_st, val));
                    }
                    else
                    {
                        dq.push_back(arg2);
                        dq.push_back(*it);
                    }
                }
                else
                {
                    dq.push_back(*it);
                }
            }
            else if(it->is_function())
            {
                evaluator_object<T> const arg = dq.back();
                if(arg.is_constant())
                {
                    dq.pop_back();
                    const T varg = arg.eval();
                    const T val = it->eval(varg);
                    std::stringstream sst;
                    sst.precision(17);
                    sst.setf(std::ios::scientific);
                    sst << val;
                    const std::string sst_st = sst.str();
                    dq.push_back(evaluator_object<T>(sst_st, val));
                }
                else
                {
                    dq.push_back(*it);
                }
            }
            else
            {
                dq.push_back(*it);
            }
        }

        if(m_expression.size() > dq.size())
        {
            m_expression.clear();
            m_expression.reserve(dq.size());
            while(!dq.empty())
            {
                m_expression.push_back(dq.front());
                dq.pop_front();
            }
            was_changed = true;
        }
        else
        {
            dq.clear();
        }

        for(typename std::vector<evaluator_object<T> >::iterator
            it = m_expression.begin(), it_end = m_expression.end(); it != it_end; ++it)
        {
            if(it->is_operator())
            {
                const evaluator_object<T> arg2 = dq.back();
                dq.pop_back();
                const evaluator_object<T> arg1 = dq.back();
                // Such things as a*0 or 0*a
                if(it->str() == "*" && ((arg2.is_constant() && arg2.eval() == static_cast<T>(0)) ||
                                        (arg1.is_constant() && arg1.eval() == static_cast<T>(0) &&
                                         !arg2.is_operator() && !arg2.is_function())))
                {
                    dq.pop_back();
                    if(arg2.is_constant() && arg2.eval() == static_cast<T>(0))
                        dq.push_back(arg2);
                    else
                        dq.push_back(arg1);
                }
                // Such things as a*1 or 1*a
                else if(it->str() == "*" && ((arg2.is_constant() && arg2.eval() == static_cast<T>(1)) ||
                                             (arg1.is_constant() && arg1.eval() == static_cast<T>(1) &&
                                              !arg2.is_operator() && !arg2.is_function())))
                {
                    dq.pop_back();
                    if(arg2.is_constant() && arg2.eval() == static_cast<T>(1))
                        dq.push_back(arg1);
                    else
                        dq.push_back(arg2);
                }
                // Such things as a+0 or 0+a
                else if(it->str() == "+" && ((arg2.is_constant() && arg2.eval() == static_cast<T>(0)) ||
                                             (arg1.is_constant() && arg1.eval() == static_cast<T>(0) &&
                                              !arg2.is_operator() && !arg2.is_function())))
                {
                    dq.pop_back();
                    if(arg2.is_constant() && arg2.eval() == static_cast<T>(0))
                        dq.push_back(arg1);
                    else
                        dq.push_back(arg2);
                }
                // Such things as a-0
                else if(it->str() == "-" && arg2.is_constant() && arg2.eval() == static_cast<T>(0))
                {
                    dq.pop_back();
                    dq.push_back(arg1);
                }
                // Nothing...
                else
                {
                    dq.push_back(arg2);
                    dq.push_back(*it);
                }
            }
            else
            {
                dq.push_back(*it);
            }
        }

        if(m_expression.size() > dq.size())
        {
            m_expression.clear();
            m_expression.reserve(dq.size());
            while(!dq.empty())
            {
                m_expression.push_back(dq.front());
                dq.pop_front();
            }
            was_changed = true;
        }
        else
        {
            dq.clear();
        }
    }
    while(was_changed);
    return true;
}

#endif // EVALUATOR_SIMPLIFY_H

