#if !defined PARSER_H
#define PARSER_H

// Usage example:
//
// #include "parser.h"
// ...
// parser<double> p;
// if(!p.parse("exp(-(0.5-x)*(0.5-x)-(0.5-z)*(0.5-z))"))
//     cerr << p.get_error() << endl;
// else if(!p.simplify()) // simplify is optional step
//     cerr << p.get_error() << endl;
// else
// {
//     if(!p.compile()) // compile is optional step
//        cerr << p.get_error() << endl;
//     p.set_const("x", 0.4);
//     p.set_const("z", 0.8);
//     double result;
//     if(!p.calculate(result))
//        cerr << p.get_error() << endl;
//     else
//         cout << result << endl;
// }

#include <map>
#include <set>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stack>
#include <deque>
#include <algorithm>
#include <cmath>
#include <utility>
#include <complex>
#include <typeinfo>
#include <limits>
#include <cctype>

//#define PARSER_ASM_DEBUG
//#define PARSER_JIT_DISABLE

#include "parser_internal.h"
#include "parser_operations.h"
#include "parser_opcodes.h"

// =================================================================================================

template<typename T>
class parser
{
protected:
    std::vector<std::string> expression;
    std::vector<parser_internal::parser_object<T> > expression_objects;
    std::map<std::string, T(*)(const T &)> functions;
    std::map<std::string, parser_internal::var_container<T> > constants;
    std::map<char, std::pair<unsigned short int, T(*)(const T &, const T &)> > operators;
    bool status;
    std::string error_string;
    bool is_compiled;
#if !defined PARSER_JIT_DISABLE
    char * volatile jit_code;
    size_t jit_code_size;
    T * volatile jit_stack;
    size_t jit_stack_size;
#endif

    // =============================================================================================

    template<typename U>
    inline T incorrect_number(const std::complex<U> &) const
    {
        U num = std::numeric_limits<U>::max();
        return static_cast<T>(std::complex<U>(num, num));
    }

    template<typename U>
    inline T incorrect_number(const U &) const
    {
        return static_cast<T>(std::numeric_limits<U>::max());
    }

    template<typename U>
    inline bool is_incorrect(const std::complex<U> & val) const
    {
        static U num = std::numeric_limits<U>::max() * (static_cast<U>(1.0) - static_cast<U>(1e-3));
        return val.real() >= num && val.imag() >= num;
    }

    template<typename U>
    inline bool is_incorrect(const U & val) const
    {
        static U num = std::numeric_limits<U>::max() * (static_cast<U>(1.0) - static_cast<U>(1e-3));
        return val >= num;
    }

    // =============================================================================================

    void init()
    {
        using namespace std;
        using namespace parser_internal;
        status = false;
#if !defined PARSER_JIT_DISABLE
        is_compiled = false;
        jit_code = NULL;
        jit_code_size = 0;
        jit_stack = NULL;
        jit_stack_size = 0;
#endif
        init_functions(functions);
        init_operators(operators);
        init_constants(constants);
    }

    // =============================================================================================

    void convert_to_objects()
    {
        using namespace std;
        using namespace parser_internal;
        expression_objects.clear();
        expression_objects.reserve(expression.size());
        for(vector<string>::const_iterator it = expression.begin(); it != expression.end(); ++it)
        {
            typename map<string, var_container<T> >::const_iterator itc = constants.find(*it);
            if(itc != constants.end() && !is_incorrect(itc->second.value()))
            {
                expression_objects.push_back(parser_object<T>(*it, itc->second.value()));
            }
            else
            {
                typename map<char, pair<unsigned short int, T(*)(const T &, const T &)> >::const_iterator ito = operators.find((*it)[0]);
                if(ito != operators.end())
                {
                    expression_objects.push_back(parser_object<T>(*it, ito->second.second));
                }
                else
                {
                    typename map<string, T(*)(const T &)>::const_iterator itf = functions.find(*it);
                    if(itf != functions.end())
                    {
                        expression_objects.push_back(parser_object<T>(*it, itf->second));
                    }
                    else
                    {
                        constants[*it].value() = incorrect_number(T());
                        expression_objects.push_back(parser_object<T>(*it, constants[*it].pointer()));
                    }
                }
            }
        }
        expression.clear();
    }

    // =============================================================================================

    void copy_from_other_parser(const parser & other)
    {
        expression = other.expression;
        expression_objects = other.expression_objects;
        functions = other.functions;
        constants = other.constants;
        operators = other.operators;
        status = other.status;
        error_string = other.error_string;
        is_compiled = false;
#if !defined PARSER_JIT_DISABLE
        jit_code = NULL;
        jit_code_size = 0;
        jit_stack = NULL;
        jit_stack_size = 0;
#endif
    }

    // =============================================================================================

public:
    parser(const parser & other)
    {
        copy_from_other_parser(other);
    }

    const parser & operator = (const parser & other)
    {
        if(this != &other)
            copy_from_other_parser(other);
        return * this;
    }

    parser()
    {
        init();
    }

    parser(const std::string & str)
    {
        init();
        parse(str);
    }

    ~parser()
    {
#if !defined PARSER_JIT_DISABLE
        if(jit_code && jit_code_size)
        {
#if defined _WIN32 || defined _WIN64
            free(jit_code);
#else
            munmap(jit_code, jit_code_size);
#endif
        }
        if(jit_stack && jit_stack_size)
        {
            delete [] jit_stack;
        }
#endif
    }

    // =============================================================================================

    inline const std::string & get_error() const
    {
        return error_string;
    }

    // =============================================================================================

    inline void set_const(const std::string & name, const T & value)
    {
        constants[name].value() = value;
    }

    // =============================================================================================

    void reset_const()
    {
        using namespace std;
        using namespace parser_internal;
        constants.clear();
        init_constants(constants);
        if(is_parsed())
            for(typename vector<parser_object<T> >::iterator it = expression_objects.begin(); it != expression_objects.end(); ++it)
                constants[it->str()].value() = incorrect_number(T());
    }

    // =============================================================================================

    inline bool is_parsed() const
    {
        return status;
    }

    // =============================================================================================

    bool parse(const std::string & str)
    {
        using namespace std;
        using namespace parser_internal;

        expression.clear();
        expression_objects.clear();
        error_string = "";

        status = true;
        bool str_begin = true;
        bool unary_minus = false;
        stack<string> st;

        for(string::const_iterator it = str.begin(); it != str.end() && status;)
        {
            char sym = *it;
            if(sym >= '0' && sym <= '9')
            {
                string a;
                while(it != str.end() && sym >= '0' && sym <= '9')
                {
                    a.push_back(sym);
                    ++it;
                    if(it != str.end())
                    {
                        sym = *it;
                    }
                }
                if(it != str.end() && (sym == '.' || sym == ','))
                {
                    a.push_back('.');
                    ++it;
                    if(it != str.end())
                    {
                        sym = *it;
                    }
                    while(it != str.end() && sym >= '0' && sym <= '9')
                    {
                        a.push_back(sym);
                        ++it;
                        if(it != str.end())
                        {
                            sym = *it;
                        }
                    }
                }
                if(it != str.end() && (sym == 'e' || sym == 'E' || sym == 'd' || sym == 'D'))
                {
                    a.push_back('e');
                    ++it;
                    if(it != str.end())
                    {
                        sym = *it;
                    }
                    if(it != str.end() && (sym == '-' || sym == '+'))
                    {
                        a.push_back(sym);
                        ++it;
                        if(it != str.end())
                        {
                            sym = *it;
                        }
                    }
                    while(it != str.end() && sym >= '0' && sym <= '9')
                    {
                        a.push_back(sym);
                        ++it;
                        if(it != str.end())
                        {
                            sym = *it;
                        }
                    }
                }

                if(unary_minus)
                {
                    a = "-" + a;
                    unary_minus = false;
                }
                stringstream b;
                b << a;
                double c;
                b >> c;
                expression.push_back(a);
                constants[a].value() = static_cast<T>(c);
                str_begin = false;
            }
            else if(sym == '(')
            {
                st.push("(");
                str_begin = true;
                ++it;
            }
            else if(sym == ')')
            {
                while(!st.empty() && st.top() != "(")
                {
                    expression.push_back(st.top());
                    st.pop();
                }
                if(st.empty())
                {
                    status = false;
                    error_string = "Wrong brackets balance!";
                    break;
                }
                if(str_begin)
                {
                    status = false;
                    error_string = "Unexpected ')'!";
                    break;
                }
                st.pop();
                if(!st.empty() && functions.find(st.top()) != functions.end())
                {
                    expression.push_back(st.top());
                    st.pop();
                }
                str_begin = false;
                ++it;
            }
            else if(operators.find(sym) != operators.end())
            {
                if(sym == '-' && str_begin)
                {
                    string::const_iterator it2 = it + 1;
                    if(it2 == str.end())
                    {
                        status = false;
                        error_string = "Unexpected '-'!";
                        break;
                    }
                    if(*it2 >= '0' && *it2 <= '9')
                    {
                        unary_minus = true;
                    }
                    else
                    {
                        constants["-1.0"].value() = static_cast<T>(-1.0);
                        expression.push_back("-1.0");
                        st.push("*");
                    }
                }
                else
                {
                    char op;
                    if(!st.empty()) op = st.top().c_str()[0];
                    while(!st.empty() && operators.find(op) != operators.end() &&
                          operators[sym].first <= operators[op].first)
                    {
                        expression.push_back(st.top());
                        st.pop();
                        if(!st.empty()) op = st.top().c_str()[0];
                    }
                    string tmp;
                    tmp.push_back(sym);
                    st.push(tmp);
                }
                str_begin = false;
                ++it;
            }
            else if(sym != ' ' && sym != '\t' && sym != '\0' && sym != '\r' && sym != '\n')
            {
                string funcname;
                while(it != str.end() && *it != '(' && *it != ')' &&
                      *it != ' ' && *it != '\t' && *it != '\0' && *it != '\r' && *it != '\n' &&
                      operators.find(*it) == operators.end())
                {
                    funcname.push_back(*it);
                    ++it;
                }
                transform(funcname.begin(), funcname.end(), funcname.begin(), ::tolower);
                if(functions.find(funcname) != functions.end())
                {
                    st.push(funcname);
                }
                else if(it == str.end() || *it != '(')
                {
                    expression.push_back(funcname);
                }
                else
                {
                    status = false;
                    error_string = "Wrong function!";
                    break;
                }
                str_begin = false;
            }
            else
            {
                ++it;
            }
        }

        while(status && !st.empty())
        {
            if(operators.find(st.top().c_str()[0]) == operators.end())
            {
                status = false;
                error_string = "Wrong expression!";
                break;
            }
            expression.push_back(st.top());
            st.pop();
        }

        if(status) convert_to_objects();
        return status;
    }

    // =============================================================================================

    bool simplify()
    {
        using namespace std;
        using namespace parser_internal;

        if(!is_parsed())
        {
            error_string = "Not parsed!";
            return false;
        }

        bool was_changed;
        do
        {
            deque<parser_object<T> > dq;
            was_changed = false;

            for(typename vector<parser_object<T> >::iterator it = expression_objects.begin(); it != expression_objects.end(); ++it)
            {
                if(it->is_operator())
                {
                    parser_object<T> arg2 = dq.back();
                    if(arg2.is_constant())
                    {
                        dq.pop_back();
                        parser_object<T> arg1 = dq.back();
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
                            dq.push_back(parser_object<T>(sst_st, val));
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
                    parser_object<T> arg = dq.back();
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
                        dq.push_back(parser_object<T>(sst_st, val));
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

            if(expression_objects.size() > dq.size())
            {
                expression_objects.clear();
                expression_objects.reserve(dq.size());
                while(!dq.empty())
                {
                    expression_objects.push_back(dq.front());
                    dq.pop_front();
                }
                was_changed = true;
            }
            else
            {
                dq.clear();
            }

            for(typename vector<parser_object<T> >::iterator it = expression_objects.begin(); it != expression_objects.end(); ++it)
            {
                if(it->is_operator())
                {
                    parser_object<T> arg2 = dq.back();
                    dq.pop_back();
                    parser_object<T> arg1 = dq.back();
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

            if(expression_objects.size() > dq.size())
            {
                expression_objects.clear();
                expression_objects.reserve(dq.size());
                while(!dq.empty())
                {
                    expression_objects.push_back(dq.front());
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

    // =============================================================================================

    bool calculate(T & result)
    {
        using namespace std;
        using namespace parser_internal;

        if(!is_parsed())
        {
            error_string = "Not parsed!";
            return false;
        }

#if !defined PARSER_JIT_DISABLE
        if(is_compiled)
        {
            typedef void(PARSER_JIT_CALL * jit_f_type)();
            jit_f_type func = reinterpret_cast<jit_f_type>(jit_code);
            func();
            result = jit_stack[0];
            return true;
        }
#endif

        //stack<T> st;
        simple_stack<T> st;

        for(typename vector<parser_object<T> >::const_iterator it = expression_objects.begin(); it != expression_objects.end(); ++it)
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

    // =============================================================================================

    bool compile();

    // =============================================================================================

    void debug_print() const
    {
        using namespace std;
        using namespace parser_internal;
        if(expression.size() > expression_objects.size())
            for(vector<string>::const_iterator it = expression.begin(); it != expression.end(); ++it)
                cout << * it << ' ';
        else
            for(typename vector<parser_object<T> >::const_iterator it = expression_objects.begin(); it != expression_objects.end(); ++it)
            {
                cout << it->str();
                if(it->is_variable())
                    cout << "->" << it->eval() << ' ';
                else
                    cout << ' ';
            }
        cout << endl;
    }
};

#include "parser_compiler.h"

#endif // PARSER_H
