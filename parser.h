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
//     p.set_var("x", 0.4);
//     p.set_var("z", 0.8);
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
#include "parser_templates.h"

// =================================================================================================

    /*
    // S := EXPR
    // SIGN := "+" | "-"
    // OPER := SIGN | "*" | "/" | "^"
    // FUNC := "abs" | "sin" | "cos" | "tan" | "log" | "exp" | "sqrt" | ...
    // EXPR := SIGN E1 | E1
    // E1 := var E2 | const E2 | FUNC "(" EXPR ") E2 | "(" EXPR ")" E2
    // E2 := OPER E1 | eps
    //
    // [00|S]***[01|EXPR]    /->[04|SIGN]***[05|E1]
    //            |          |                 |
    //            V          |                 *------------------------------------\
    //     /-->[02|EXPR|#1]--/                 |                                    |
    //     |   [03|EXPR|#2]---->[06|E1]--------|    /->[11|var]***[12|E2]-----\     |
    //     |                                   |    |                         |     |
    //     \------\                            V    | /->[13|const]***[14|E2]-*     |
    //            |                     [07|E1|#1]--/ |                       |     |
    //            |                     [08|E1|#2]----/    /------------------*     |
    //            |           /---------[09|E1|#3]         |                  |     |
    //            |           |         [10|E1|#4]         |                  |     |
    //            |           V             |              V                  |     |
    //            |        [15|"("]         V         [24|E2|#1]------\       |     |
    //            |          ***        [19|func]     [25|E2|#2]      |       |     |
    //            *--------[16|EXPR]       ***             |          V       |     |
    //            |          ***         [20|"("]          V      [27|OPER]   |     |
    //            |        [17|")"]        ***          [26|eps]     ***      |     |
    //            |          ***        [21|EXPR]---\              [28|E1]    |     |
    //            |        [18|E2]         ***      |                 |       |     |
    //            |           |          [22|")"]   |                 \-------)-----/
    //            |           |            ***      |                         |
    //            |           |          [23|E2]    |                         |
    //            |           |             |       |                         |
    //            |           \-------------*-------)-------------------------/
    //            |                                 |
    //            \---------------------------------/
    //
    // +----+------------------------+------+--------+-------+--------+-------+
    // | N  |  Terminals             | Jump | Accept | Stack | Return | Error |
    // +----+------------------------+------+--------+-------+--------+-------+
    // | 00 |  func var const sign ( |  01  |   0    |   0   |    0   |   1   |
    // | 01 |  func var const sign ( |  02  |   0    |   0   |    0   |   1   |
    // | 02 |  sign                  |  04  |   0    |   0   |    0   |   0   |
    // | 03 |  func var const (      |  06  |   0    |   0   |    0   |   1   |
    // | 04 |  sign                  |  05  |   1    |   0   |    0   |   1   |
    // | 05 |  func var const (      |  07  |   0    |   0   |    0   |   1   |
    // | 06 |  func var const (      |  07  |   0    |   0   |    0   |   1   |
    // | 07 |  var                   |  11  |   0    |   0   |    0   |   0   |
    // | 08 |  const                 |  13  |   0    |   0   |    0   |   0   |
    // | 09 |  (                     |  15  |   0    |   0   |    0   |   0   |
    // | 10 |  func                  |  19  |   0    |   0   |    0   |   1   |
    // | 11 |  var                   |  12  |   1    |   0   |    0   |   1   |
    // | 12 |  oper eps              |  24  |   0    |   0   |    0   |   1   |
    // | 13 |  const                 |  14  |   1    |   0   |    0   |   1   |
    // | 14 |  oper eps              |  24  |   0    |   0   |    0   |   1   |
    // | 15 |  (                     |  16  |   1    |   0   |    0   |   1   |
    // | 16 |  func var const sign ( |  02  |   0    |   1   |    0   |   1   |
    // | 17 |  )                     |  18  |   1    |   0   |    0   |   1   |
    // | 18 |  oper eps              |  24  |   0    |   0   |    0   |   1   |
    // | 19 |  func                  |  20  |   1    |   0   |    0   |   1   |
    // | 20 |  (                     |  21  |   1    |   0   |    0   |   1   |
    // | 21 |  func var const sign ( |  02  |   0    |   1   |    0   |   1   |
    // | 22 |  )                     |  23  |   1    |   0   |    0   |   1   |
    // | 23 |  oper eps              |  24  |   0    |   0   |    0   |   1   |
    // | 24 |  oper                  |  27  |   0    |   0   |    0   |   0   |
    // | 25 |  eps                   |  26  |   0    |   0   |    0   |   1   |
    // | 26 |  eps                   |  -1  |   0    |   0   |    1   |   1   |
    // | 27 |  oper                  |  28  |   1    |   0   |    0   |   1   |
    // | 28 |  func var const (      |  07  |   0    |   0   |    0   |   1   |
    // +----+------------------------+------+--------+-------+--------+-------+
    */

// =================================================================================================

template<typename T>
class parser
{
protected:
    std::vector<parser_internal::parser_table_record> parser_table;
    std::vector<parser_internal::parser_object<T> > expression;
    std::map<std::string, T(*)(const T &)> functions;
    std::map<std::string, parser_internal::var_container<T> > variables;
    std::map<char, std::pair<unsigned short int, T(*)(const T &, const T &)> > operators;
    bool status;
    std::string error_string;
#if !defined PARSER_JIT_DISABLE
    bool is_compiled;
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
        init_variables(variables);

        parser_table.resize(29);
        parser_table[ 0].set_values("func var const sign (",  1, false, false, false, true );
        parser_table[ 1].set_values("func var const sign (",  2, false, false, false, true );
        parser_table[ 2].set_values("sign",                   4, false, false, false, false);
        parser_table[ 3].set_values("func var const (",       6, false, false, false, true );
        parser_table[ 4].set_values("sign",                   5, true,  false, false, true );
        parser_table[ 5].set_values("func var const (",       7, false, false, false, true );
        parser_table[ 6].set_values("func var const (",       7, false, false, false, true );
        parser_table[ 7].set_values("var",                   11, false, false, false, false);
        parser_table[ 8].set_values("const",                 13, false, false, false, false);
        parser_table[ 9].set_values("(",                     15, false, false, false, false);
        parser_table[10].set_values("func",                  19, false, false, false, true );
        parser_table[11].set_values("var",                   12, true,  false, false, true );
        parser_table[12].set_values("oper eps",              24, false, false, false, true );
        parser_table[13].set_values("const",                 14, true,  false, false, true );
        parser_table[14].set_values("oper eps",              24, false, false, false, true );
        parser_table[15].set_values("(",                     16, true,  false, false, true );
        parser_table[16].set_values("func var const sign (",  2, false, true,  false, true );
        parser_table[17].set_values(")",                     18, true,  false, false, true );
        parser_table[18].set_values("oper eps",              24, false, false, false, true );
        parser_table[19].set_values("func",                  20, true,  false, false, true );
        parser_table[20].set_values("(",                     21, true,  false, false, true );
        parser_table[21].set_values("func var const sign (",  2, false, true,  false, true );
        parser_table[22].set_values(")",                     23, true,  false, false, true );
        parser_table[23].set_values("oper eps",              24, false, false, false, true );
        parser_table[24].set_values("oper",                  27, false, false, false, false);
        parser_table[25].set_values("eps",                   26, false, false, false, true );
        parser_table[26].set_values("eps",                   -1, false, false, true,  true );
        parser_table[27].set_values("oper",                  28, true,  false, false, true );
        parser_table[28].set_values("func var const (",       7, false, false, false, true );
    }

    // =============================================================================================

    void copy_from_other_parser(const parser & other)
    {
        expression = other.expression;
        functions = other.functions;
        variables = other.variables;
        operators = other.operators;
        status = other.status;
        error_string = other.error_string;
        parser_table = other.parser_table;
#if !defined PARSER_JIT_DISABLE
        is_compiled = false;
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

    inline void set_var(const std::string & name, const T & value)
    {
        variables[name].value() = value;
    }

    // =============================================================================================

    void reset_const()
    {
        using namespace std;
        using namespace parser_internal;
        variables.clear();
        init_variables(variables);
        if(is_parsed())
            for(typename vector<parser_object<T> >::iterator
                it = expression.begin(); it != expression.end(); ++it)
                variables[it->str()].value() = incorrect_number(T());
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
        error_string.clear();
        status = true;

        vector<string> tokens;
        for(string::const_iterator it = str.begin(); it != str.end();)
        {
            string a;
            if(*it >= '0' && *it <= '9')
            {
                while(it != str.end() && *it >= '0' && *it <= '9')
                {
                    a.push_back(*(it++));
                }
                if(it != str.end() && (*it == '.' || *it == ','))
                {
                    a.push_back('.');
                    while((++it) != str.end() && *it >= '0' && *it <= '9')
                    {
                        a.push_back(*it);
                    }
                }
                if(it != str.end() && (*it == 'e' || *it == 'E' || *it == 'd' || *it == 'D'))
                {
                    a.push_back('e');
                    if((++it) != str.end() && (*it == '-' || *it == '+'))
                    {
                        a.push_back(*(it++));
                    }
                    while(it != str.end() && *it >= '0' && *it <= '9')
                    {
                        a.push_back(*(it++));
                    }
                }
                transform(a.begin(), a.end(), a.begin(), ::tolower);
                tokens.push_back(a);
            }
            else if((*it >= 'a' && *it <= 'z') || (*it >= 'A' && *it <= 'Z'))
            {
                while(it != str.end() && *it != '(' && *it != ')' && *it != '\f' && *it != '\v' &&
                      *it != ' ' && *it != '\t' && *it != '\0' && *it != '\r' && *it != '\n' &&
                      operators.find(*it) == operators.end())
                {
                    a.push_back(*(it++));
                }
                transform(a.begin(), a.end(), a.begin(), ::tolower);
                tokens.push_back(a);
            }
            else if(operators.find(*it) != operators.end() || *it == '(' || *it == ')')
            {
                a.push_back(*(it++));
                transform(a.begin(), a.end(), a.begin(), ::tolower);
                tokens.push_back(a);
            }
            else if(*it == ' ' || *it == '\t' || *it == '\0' || *it == '\r' ||
                    *it == '\n' || *it == '\f' || *it == '\v')
            {
                ++it;
            }
            else
            {
                status = false;
                error_string = string("Unexpected symbol `") + string().assign(1, *it) + string("`!");
                return false;
            }
        }

        if(tokens.size() <= 0)
        {
            status = false;
            error_string = "No tokens!";
            return false;
        }

        enum token_type
        {
            TTYPE_EPS,
            TTYPE_SIGN_MINUS,
            TTYPE_SIGN_PLUS,
            TTYPE_FUNC,
            TTYPE_OPER,
            TTYPE_VAR,
            TTYPE_CONST,
            TTYPE_BR_OPEN,
            TTYPE_BR_CLOSE
        };
        token_type ttype_curr = TTYPE_EPS;
        bool unary_minus = false;
        stack<string> st;

        size_t token_pos_curr = 0;
        size_t table_pos_curr = 0;
        stack<size_t> table_stack;
        for(bool flag_continue = true; flag_continue;)
        {
            bool good_token = false;
            for(vector<string>::const_iterator it = parser_table[table_pos_curr].Terminals.begin();
                !good_token && it != parser_table[table_pos_curr].Terminals.end(); ++it)
            {
                if(token_pos_curr < tokens.size())
                {
                    if(*it == "eps")
                    {
                        if(tokens[token_pos_curr] == ")")
                        {
                            good_token = true;
                            ttype_curr = TTYPE_EPS;
                        }
                    }
                    else if(*it == "func")
                    {
                        if(functions.find(tokens[token_pos_curr]) != functions.end())
                        {
                            good_token = true;
                            ttype_curr = TTYPE_FUNC;
                        }
                    }
                    else if(*it == "oper")
                    {
                        if(operators.find(tokens[token_pos_curr][0]) != operators.end())
                        {
                            good_token = true;
                            ttype_curr = TTYPE_OPER;
                        }
                    }
                    else if(*it == "var")
                    {
                        if(tokens[token_pos_curr][0] >= 'a' && tokens[token_pos_curr][0] <= 'z' &&
                           ((token_pos_curr + 1 >= tokens.size()) ||
                            (operators.find(tokens[token_pos_curr + 1][0]) != operators.end()) ||
                            (tokens[token_pos_curr + 1] == ")")))
                        {
                            good_token = true;
                            ttype_curr = TTYPE_VAR;
                        }
                    }
                    else if(*it == "const")
                    {
                        if(tokens[token_pos_curr][0] >= '0' && tokens[token_pos_curr][0] <= '9' &&
                           ((token_pos_curr + 1 >= tokens.size()) ||
                            (operators.find(tokens[token_pos_curr + 1][0]) != operators.end()) ||
                            (tokens[token_pos_curr + 1] == ")")))
                        {
                            good_token = true;
                            ttype_curr = TTYPE_CONST;
                        }
                    }
                    else if(*it == "sign")
                    {
                        if(tokens[token_pos_curr][0] == '-' || tokens[token_pos_curr][0] == '+')
                        {
                            good_token = true;
                            if(tokens[token_pos_curr][0] == '-')
                            {
                                ttype_curr = TTYPE_SIGN_MINUS;
                                unary_minus = true;
                            }
                            else
                                ttype_curr = TTYPE_SIGN_PLUS;
                        }
                    }
                    else if(*it == tokens[token_pos_curr])
                    {
                        good_token = true;
                        if(tokens[token_pos_curr][0] == '(')
                            ttype_curr = TTYPE_BR_OPEN;
                        else if(tokens[token_pos_curr][0] == ')')
                            ttype_curr = TTYPE_BR_CLOSE;
                    }
                }
                else if(*it == "eps")
                {
                    good_token = true;
                    ttype_curr = TTYPE_EPS;
                }
            }

            if(good_token)
            {
                if(parser_table[table_pos_curr].Stack)
                    table_stack.push(table_pos_curr + 1);
                if(parser_table[table_pos_curr].Accept)
                {
                    switch(ttype_curr)
                    {
                    case TTYPE_CONST:
                    {
                        string a = tokens[token_pos_curr];
                        if(unary_minus)
                        {
                            a = "-" + a;
                            unary_minus = false;
                        }
                        stringstream b;
                        b << a;
                        T c;
                        b >> c;
                        expression.push_back(parser_object<T>(a, c));
                        break;
                    }
                    case TTYPE_VAR:
                    {
                        string a = tokens[token_pos_curr];
                        if(unary_minus)
                        {
                            T m_one = static_cast<T>(-1);
                            expression.push_back(parser_object<T>("-1", m_one));
                            st.push("*");
                            unary_minus = false;
                        }
                        typename map<string, var_container<T> >::const_iterator itc = variables.find(a);
                        if(itc == variables.end())
                        {
                            variables[a].value() = incorrect_number(T());
                            itc = variables.find(a);
                        }
                        expression.push_back(parser_object<T>(a, itc->second.pointer()));
                        break;
                    }
                    case TTYPE_BR_OPEN:
                    case TTYPE_FUNC:
                    {
                        if(unary_minus)
                        {
                            T m_one = static_cast<T>(-1);
                            expression.push_back(parser_object<T>("-1", m_one));
                            st.push("*");
                            unary_minus = false;
                        }
                        st.push(tokens[token_pos_curr]);
                        break;
                    }
                    case TTYPE_OPER:
                    {
                        char op, sym = tokens[token_pos_curr][0];
                        if(!st.empty()) op = st.top()[0];
                        while(!st.empty() && operators.find(op) != operators.end() &&
                              operators[sym].first <= operators[op].first)
                        {
                            expression.push_back(parser_object<T>(st.top(),
                                                         operators.find(st.top()[0])->second.second));
                            st.pop();
                            if(!st.empty()) op = st.top()[0];
                        }
                        st.push(tokens[token_pos_curr]);
                        break;
                    }
                    case TTYPE_BR_CLOSE:
                    {
                        while(!st.empty() && st.top() != "(")
                        {
                            expression.push_back(parser_object<T>(st.top(),
                                                         operators.find(st.top()[0])->second.second));
                            st.pop();
                        }
                        if(st.empty())
                        {
                            status = false;
                            error_string = "Wrong brackets balance!";
                            return false;
                        }
                        st.pop();
                        if(!st.empty() && functions.find(st.top()) != functions.end())
                        {
                            expression.push_back(parser_object<T>(st.top(),
                                                         functions.find(st.top())->second));
                            st.pop();
                        }
                        break;
                    }
                    default:
                        break;
                    }
                    token_pos_curr++;
                }
                if(parser_table[table_pos_curr].Return)
                {
                    if(table_stack.size() > 0)
                    {
                        table_pos_curr = table_stack.top();
                        table_stack.pop();
                    }
                    else
                        flag_continue = false;
                }
                else
                    table_pos_curr = (size_t)parser_table[table_pos_curr].Jump;
            }
            else
            {
                if(parser_table[table_pos_curr].Error)
                {
                    status = false;
                    if(token_pos_curr < tokens.size())
                        error_string = string("Bad token `") + tokens[token_pos_curr] + string("`!");
                    else
                        error_string = "Unexpected end of string!";
                    return false;
                }
                else
                {
                    table_pos_curr++;
                }
            }
        }

        while(status && !st.empty())
        {
            if(operators.find(st.top()[0]) == operators.end())
            {
                status = false;
                error_string = "Wrong expression!";
                break;
            }
            expression.push_back(parser_object<T>(st.top(),
                                         operators.find(st.top()[0])->second.second));
            st.pop();
        }

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

            for(typename vector<parser_object<T> >::iterator
                it = expression.begin(); it != expression.end(); ++it)
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

            for(typename vector<parser_object<T> >::iterator
                it = expression.begin(); it != expression.end(); ++it)
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
            jit_f_type func = NULL;
            size_t call_addr = (size_t)(& func);
            size_t code_addr = (size_t)(& jit_code);
            memcpy((void *)call_addr, (void *)code_addr, sizeof(void *));
            func();
            result = jit_stack[0];
            return true;
        }
#endif

        //stack<T> st;
        simple_stack<T> st;

        for(typename vector<parser_object<T> >::const_iterator
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

    // =============================================================================================

    bool compile_inline();
    bool compile_extcall();

    inline bool compile()
    {
        return compile_extcall();
    }

    // =============================================================================================

    void debug_print() const
    {
        using namespace std;
        using namespace parser_internal;
        for(typename vector<parser_object<T> >::const_iterator
            it = expression.begin(); it != expression.end(); ++it)
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

#include "parser_compiler_inline.h"
#include "parser_compiler_extcall.h"

#endif // PARSER_H
