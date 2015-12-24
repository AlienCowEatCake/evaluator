#include "parser.h"
#include <iostream>
#include <sstream>
#include <stack>
#include <deque>
#include <algorithm>
#include <cmath>
#include <utility>

using namespace std;

#if defined(_MSC_VER) && _MSC_VER < 1800
// http://functions.wolfram.com/ElementaryFunctions/ArcCosh/02/
double acosh(double x) { return log(x + sqrt(x * x - 1)); }
// http://functions.wolfram.com/ElementaryFunctions/ArcSinh/02/
double asinh(double x) { return log(x + sqrt(x * x + 1)); }
// http://functions.wolfram.com/ElementaryFunctions/ArcTanh/02/
double atanh(double x) { return  0.5 * log((1.0 + x) / (1.0 - x)); }
#endif

void parser::init()
{
    functions.insert("sin");
    functions.insert("cos");
    functions.insert("tan");
    functions.insert("asin");
    functions.insert("acos");
    functions.insert("atan");
    functions.insert("sinh");
    functions.insert("cosh");
    functions.insert("tanh");
    functions.insert("asinh");
    functions.insert("acosh");
    functions.insert("atanh");
    functions.insert("log");
    functions.insert("log10");
    functions.insert("abs");
    functions.insert("exp");
    functions.insert("sqrt");

    operators['+'] = 1;
    operators['-'] = 1;
    operators['*'] = 2;
    operators['/'] = 2;
    operators['^'] = 3;
}

void parser::init_const()
{
    constants["pi"] = 3.14159265358979323846264338327950;
    constants["e"]  = 2.71828182845904523536028747135266;
}

double parser::calc_operator(const string & oper, const double larg, const double rarg) const
{
    char op = oper[0];
    if(op == '+')
        return larg + rarg;
    if(op == '-')
        return larg - rarg;
    if(op == '*')
        return larg * rarg;
    if(op == '/')
        return larg / rarg;
    if(op == '^')
        return pow(larg, rarg);
    cerr << "Internal error: Unknown operator " << oper << endl;
    return 0.0;
}

double parser::calc_function(const string & func, const double arg) const
{
    if(func == "sin")
        return sin(arg);
    if(func == "cos")
        return cos(arg);
    if(func == "tan")
        return tan(arg);
    if(func == "asin")
        return asin(arg);
    if(func == "acos")
        return acos(arg);
    if(func == "atan")
        return atan(arg);
    if(func == "sinh")
        return sinh(arg);
    if(func == "cosh")
        return cosh(arg);
    if(func == "tanh")
        return tanh(arg);
    if(func == "asinh")
        return asinh(arg);
    if(func == "acosh")
        return acosh(arg);
    if(func == "atanh")
        return atanh(arg);
    if(func == "log")
        return log(arg);
    if(func == "log10")
        return log10(arg);
    if(func == "abs")
        return fabs(arg);
    if(func == "sqrt")
        return sqrt(arg);
    if(func == "exp")
        return exp(arg);
    cerr << "Internal error: Unknown function " << func << endl;
    return 0.0;
}

parser::parser()
{
    init();
    init_const();
    status = false;
}

parser::parser(const string & str)
{
    init();
    init_const();
    parse(str);
}

string parser::get_error() const
{
    return error_string;
}

void parser::set_const(const string & name, const double value)
{
    constants[name] = value;
}

void parser::reset_const()
{
    constants.clear();
    init_const();
}

bool parser::is_parsed() const
{
    return status;
}

void parser::debug_print() const
{
    for(vector<string>::const_iterator it = expression.begin(); it != expression.end(); it++)
        cout << *it << ' ';
    cout << endl;
}

bool parser::parse(const string & str)
{
    expression.clear();
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
                it++;
                sym = *it;
            }
            if(it != str.end() && (sym == '.' || sym == ','))
            {
                a.push_back('.');
                it++;
                sym = *it;
                while(it != str.end() && sym >= '0' && sym <= '9')
                {
                    a.push_back(sym);
                    it++;
                    sym = *it;
                }
            }
            if(it != str.end() && (sym == 'e' || sym == 'E' || sym == 'd' || sym == 'D'))
            {
                a.push_back('e');
                it++;
                sym = *it;
                if(it != str.end() && (sym == '-' || sym == '+'))
                {
                    a.push_back(sym);
                    it++;
                    sym = *it;
                }
                while(it != str.end() && sym >= '0' && sym <= '9')
                {
                    a.push_back(sym);
                    it++;
                    sym = *it;
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
            constants[a] = c;
            str_begin = false;
        }
        else if(sym == '(')
        {
            st.push("(");
            str_begin = true;
            it++;
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
            it++;
        }
        else if(operators.find(sym) != operators.end())
        {
            if(sym == '-' && str_begin)
            {
                string::const_iterator it2 = it + 1;
                if(*it2 >= '0' && *it2 <= '9')
                {
                    unary_minus = true;
                }
                else
                {
                    constants["-1.0"] = -1.0;
                    expression.push_back("-1.0");
                    st.push("*");
                }
            }
            else
            {
                char op;
                if(!st.empty()) op = st.top().c_str()[0];
                while(!st.empty() && operators.find(op) != operators.end() &&
                      operators[sym] <= operators[op])
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
            it++;
        }
        else if(sym != ' ' && sym != '\t' && sym != '\0' && sym != '\r' && sym != '\n')
        {
            string funcname;
            while(it != str.end() && *it != '(' && *it != ')' &&
                  *it != ' ' && *it != '\t' && *it != '\0' && *it != '\r' && *it != '\n' &&
                  operators.find(*it) == operators.end())
            {
                funcname.push_back(*it);
                it++;
            }
            transform(funcname.begin(), funcname.end(), funcname.begin(), ::tolower);
            if(functions.find(funcname) != functions.end())
            {
                st.push(funcname);
            }
            else if(*it != '(')
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
            it++;
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

    return status;
}

bool parser::simplify()
{
    if(!is_parsed())
    {
        error_string = "Not parsed!";
        return false;
    }

    bool was_changed;
    do
    {
        deque<string> dq;
        was_changed = false;

        for(vector<string>::iterator it = expression.begin(); it != expression.end(); it++)
        {
            if(constants.find(*it) == constants.end() && operators.find((*it)[0]) != operators.end())
            {
                string arg2 = dq.back();
                if(constants.find(arg2) != constants.end())
                {
                    dq.pop_back();
                    string arg1 = dq.back();
                    if(constants.find(arg1) != constants.end())
                    {
                        dq.pop_back();
                        double val = calc_operator(*it, constants[arg1], constants[arg2]);
                        stringstream sst;
                        sst.precision(17);
                        sst.setf(ios::scientific);
                        sst << val;
                        string sst_st = sst.str();
                        constants[sst_st] = val;
                        dq.push_back(sst_st);
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
            else if(functions.find(*it) != functions.end())
            {
                string arg = dq.back();
                if(constants.find(arg) != constants.end())
                {
                    dq.pop_back();
                    double val = calc_function(*it, constants[arg]);
                    stringstream sst;
                    sst.precision(17);
                    sst.setf(ios::scientific);
                    sst << val;
                    string sst_st = sst.str();
                    constants[sst_st] = val;
                    dq.push_back(sst_st);
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

        for(vector<string>::iterator it = expression.begin(); it != expression.end(); it++)
        {
            if(constants.find(*it) == constants.end() && operators.find((*it)[0]) != operators.end())
            {
                string arg2 = dq.back();
                dq.pop_back();
                string arg1 = dq.back();
                // Such things as a*0 or 0*a
                if(*it == "*" && ((constants.find(arg2) != constants.end() && constants[arg2] == 0.0) ||
                                  (constants.find(arg1) != constants.end() && constants[arg1] == 0.0 &&
                                   operators.find(arg2[0]) == operators.end() &&
                                   functions.find(arg2) == functions.end())))
                {
                    dq.pop_back();
                    if(constants.find(arg2) != constants.end() && constants[arg2] == 0.0)
                        dq.push_back(arg2);
                    else
                        dq.push_back(arg1);
                }
                // Such things as a*1 or 1*a
                else if(*it == "*" && ((constants.find(arg2) != constants.end() && constants[arg2] == 1.0) ||
                                       (constants.find(arg1) != constants.end() && constants[arg1] == 1.0 &&
                                        operators.find(arg2[0]) == operators.end() &&
                                        functions.find(arg2) == functions.end())))
                {
                    dq.pop_back();
                    if(constants.find(arg2) != constants.end() && constants[arg2] == 1.0)
                        dq.push_back(arg1);
                    else
                        dq.push_back(arg2);
                }
                // Such things as a+0 or 0+a
                else if(*it == "+" && ((constants.find(arg2) != constants.end() && constants[arg2] == 0.0) ||
                                       (constants.find(arg1) != constants.end() && constants[arg1] == 0.0 &&
                                        operators.find(arg2[0]) == operators.end() &&
                                        functions.find(arg2) == functions.end())))
                {
                    dq.pop_back();
                    if(constants.find(arg2) != constants.end() && constants[arg2] == 0.0)
                        dq.push_back(arg1);
                    else
                        dq.push_back(arg2);
                }
                // Such things as a-0
                else if(*it == "-" && constants.find(arg2) != constants.end() && constants[arg2] == 0.0)
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

bool parser::calculate(double & result) const
{
    if(!is_parsed())
    {
        error_string = "Not parsed!";
        return false;
    }

    stack<double> st;

    for(vector<string>::const_iterator it = expression.begin(); it != expression.end(); it++)
    {
        if(constants.find(*it) != constants.end())
        {
            st.push((*constants.find(*it)).second);
        }
        else if(operators.find((*it)[0]) != operators.end())
        {
            double arg2 = st.top();
            st.pop();
            double arg1 = st.top();
            st.pop();
            st.push(calc_operator(*it, arg1, arg2));
        }
        else if(functions.find(*it) != functions.end())
        {
            double arg = st.top();
            st.pop();
            st.push(calc_function(*it, arg));
        }
        else
        {
            error_string = "Constants must be defined!";
            return false;
        }
    }

    if(st.size() != 1)
    {
        error_string = "Internal error!";
        cerr << "Internal error: Stack size equal " << st.size() << endl;
        return false;
    }
    result = st.top();
    return true;
}
