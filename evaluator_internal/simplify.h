#ifndef SIMPLIFY_H
#define SIMPLIFY_H

#include <deque>
#include <sstream>
#include <vector>
#include <string>
#include "../evaluator.h"

// Simplify current expression
template<typename T>
bool evaluator<T>::simplify()
{
    using namespace std;
    using namespace evaluator_internal;

    if(!is_parsed())
    {
        error_string = "Not parsed!";
        return false;
    }

    bool was_changed;
    do
    {
        deque<evaluator_object<T> > dq;
        was_changed = false;

        for(typename vector<evaluator_object<T> >::iterator
            it = expression.begin(); it != expression.end(); ++it)
        {
            if(it->is_operator())
            {
                evaluator_object<T> arg2 = dq.back();
                if(arg2.is_constant())
                {
                    dq.pop_back();
                    evaluator_object<T> arg1 = dq.back();
                    if(arg1.is_constant())
                    {
                        dq.pop_back();
                        T varg1 = arg1.eval();
                        T varg2 = arg2.eval();
                        T val = it->eval(varg1, varg2);
                        stringstream sst;
                        sst.precision(17);
                        sst.setf(ios::scientific);
                        sst << val;
                        string sst_st = sst.str();
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
                evaluator_object<T> arg = dq.back();
                if(arg.is_constant())
                {
                    dq.pop_back();
                    T varg = arg.eval();
                    T val = it->eval(varg);
                    stringstream sst;
                    sst.precision(17);
                    sst.setf(ios::scientific);
                    sst << val;
                    string sst_st = sst.str();
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

        if(expression.size() > dq.size())
        {
            expression.clear();
            expression.reserve(dq.size());
            while(!dq.empty())
            {
                expression.push_back(dq.front());
                dq.pop_front();
            }
            was_changed = true;
        }
        else
        {
            dq.clear();
        }

        for(typename vector<evaluator_object<T> >::iterator
            it = expression.begin(); it != expression.end(); ++it)
        {
            if(it->is_operator())
            {
                evaluator_object<T> arg2 = dq.back();
                dq.pop_back();
                evaluator_object<T> arg1 = dq.back();
                // Such things as a*0 or 0*a
                if(it->str() == "*" && ((arg2.is_constant() && arg2.eval() == static_cast<T>(0.0)) ||
                                        (arg1.is_constant() && arg1.eval() == static_cast<T>(0.0) &&
                                         !arg2.is_operator() && !arg2.is_function())))
                {
                    dq.pop_back();
                    if(arg2.is_constant() && arg2.eval() == static_cast<T>(0.0))
                        dq.push_back(arg2);
                    else
                        dq.push_back(arg1);
                }
                // Such things as a*1 or 1*a
                else if(it->str() == "*" && ((arg2.is_constant() && arg2.eval() == static_cast<T>(1.0)) ||
                                             (arg1.is_constant() && arg1.eval() == static_cast<T>(1.0) &&
                                              !arg2.is_operator() && !arg2.is_function())))
                {
                    dq.pop_back();
                    if(arg2.is_constant() && arg2.eval() == static_cast<T>(1.0))
                        dq.push_back(arg1);
                    else
                        dq.push_back(arg2);
                }
                // Such things as a+0 or 0+a
                else if(it->str() == "+" && ((arg2.is_constant() && arg2.eval() == static_cast<T>(0.0)) ||
                                             (arg1.is_constant() && arg1.eval() == static_cast<T>(0.0) &&
                                              !arg2.is_operator() && !arg2.is_function())))
                {
                    dq.pop_back();
                    if(arg2.is_constant() && arg2.eval() == static_cast<T>(0.0))
                        dq.push_back(arg1);
                    else
                        dq.push_back(arg2);
                }
                // Such things as a-0
                else if(it->str() == "-" && arg2.is_constant() && arg2.eval() == static_cast<T>(0.0))
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

        if(expression.size() > dq.size())
        {
            expression.clear();
            expression.reserve(dq.size());
            while(!dq.empty())
            {
                expression.push_back(dq.front());
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

#endif // SIMPLIFY_H

