#ifndef EVALUATOR_PARSE_H
#define EVALUATOR_PARSE_H

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

#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <stack>
#include <map>
#include <cstring>
#include "../evaluator.h"

// Parse string 'str'
template<typename T>
bool evaluator<T>::parse(const std::string & str)
{
    using namespace std;
    using namespace evaluator_internal;

    m_expression.clear();
    m_error_string.clear();
    m_status = true;
#if !defined(EVALUATOR_JIT_DISABLE)
    m_is_compiled = false;
#endif

    vector<string> tokens;
    for(string::const_iterator it = str.begin(), it_end = str.end(); it != it_end;)
    {
        string a;
        if((*it >= '0' && *it <= '9') || *it == '.' || *it == ',')
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
                  m_operators.find(*it) == m_operators.end())
            {
                a.push_back(*(it++));
            }
            string b(a);
            transform(a.begin(), a.end(), a.begin(), ::tolower);
            if(m_functions.find(a) != m_functions.end() || m_constants.find(a) != m_constants.end())
                tokens.push_back(a);
            else
                tokens.push_back(b);
        }
        else if(m_operators.find(*it) != m_operators.end() || *it == '(' || *it == ')')
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
            m_status = false;
            m_error_string = string("Unexpected symbol `") + string().assign(1, *it) + string("`!");
            return false;
        }
    }

    if(tokens.size() <= 0)
    {
        m_status = false;
        m_error_string = "No tokens!";
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
        for(vector<string>::const_iterator it = m_transition_table[table_pos_curr].Terminals.begin(),
            it_end = m_transition_table[table_pos_curr].Terminals.end(); !good_token && it != it_end; ++it)
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
                    if(m_functions.find(tokens[token_pos_curr]) != m_functions.end())
                    {
                        good_token = true;
                        ttype_curr = TTYPE_FUNC;
                    }
                }
                else if(*it == "oper")
                {
                    if(m_operators.find(tokens[token_pos_curr][0]) != m_operators.end())
                    {
                        good_token = true;
                        ttype_curr = TTYPE_OPER;
                    }
                }
                else if(*it == "var" || *it == "const")
                {
                    bool last_token = token_pos_curr + 1 >= tokens.size();
                    const char * curr_tok = tokens[token_pos_curr].c_str();
                    const char * next_tok = (last_token ? NULL : tokens[token_pos_curr + 1].c_str());

                    bool first_letter = (curr_tok[0] >= 'a' && curr_tok[0] <= 'z') ||
                                        (curr_tok[0] >= 'A' && curr_tok[0] <= 'Z');
                    bool first_number = curr_tok[0] >= '0' && curr_tok[0] <= '9';
                    bool first_dot = curr_tok[0] == '.';
                    bool second_number = curr_tok[1] >= '0' && curr_tok[1] <= '9';
                    bool next_operator = !last_token && m_operators.find(next_tok[0]) != m_operators.end();
                    bool next_bracket = !last_token && next_tok[0] == ')';
                    bool is_constant = m_constants.find(curr_tok) != m_constants.end();

                    if(*it == "var")
                    {
                        if(!is_constant && first_letter && (last_token || next_operator || next_bracket))
                        {
                            good_token = true;
                            ttype_curr = TTYPE_VAR;
                        }
                    }
                    else if(*it == "const")
                    {
                        if((is_constant || first_number || (first_dot && second_number)) &&
                           (last_token || next_operator || next_bracket))
                        {
                            good_token = true;
                            ttype_curr = TTYPE_CONST;
                        }
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
            if(m_transition_table[table_pos_curr].Stack)
                table_stack.push(table_pos_curr + 1);
            if(m_transition_table[table_pos_curr].Accept)
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
                    T c;
                    typename std::map<std::string, T>::const_iterator
                            it = m_constants.find(tokens[token_pos_curr]);
                    if(it == m_constants.end())
                    {
                        stringstream b(a);
                        b >> c;
                    }
                    else
                        c = it->second;
                    m_expression.push_back(evaluator_object<T>(a, c));
                    break;
                }
                case TTYPE_VAR:
                {
                    string a = tokens[token_pos_curr];
                    if(unary_minus)
                    {
                        T m_one = static_cast<T>(-1);
                        m_expression.push_back(evaluator_object<T>("-1", m_one));
                        st.push("*");
                        unary_minus = false;
                    }
                    typename map<string, var_container<T> >::const_iterator itc = m_variables.find(a);
                    if(itc == m_variables.end())
                    {
                        m_variables[a].value() = incorrect_number(T());
                        itc = m_variables.find(a);
                    }
                    m_expression.push_back(evaluator_object<T>(a, itc->second.pointer()));
                    break;
                }
                case TTYPE_BR_OPEN:
                case TTYPE_FUNC:
                {
                    if(unary_minus)
                    {
                        T m_one = static_cast<T>(-1);
                        m_expression.push_back(evaluator_object<T>("-1", m_one));
                        st.push("*");
                        unary_minus = false;
                    }
                    st.push(tokens[token_pos_curr]);
                    break;
                }
                case TTYPE_OPER:
                {
                    char op = '\0', sym = tokens[token_pos_curr][0];
                    if(!st.empty()) op = st.top()[0];
                    while(!st.empty() && m_operators.find(op) != m_operators.end() &&
                          m_operators[sym].first <= m_operators[op].first)
                    {
                        m_expression.push_back(evaluator_object<T>(st.top(),
                                                     m_operators.find(st.top()[0])->second.second));
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
                        m_expression.push_back(evaluator_object<T>(st.top(),
                                                     m_operators.find(st.top()[0])->second.second));
                        st.pop();
                    }
                    if(st.empty())
                    {
                        m_status = false;
                        m_error_string = "Wrong brackets balance!";
                        return false;
                    }
                    st.pop();
                    if(!st.empty() && m_functions.find(st.top()) != m_functions.end())
                    {
                        m_expression.push_back(evaluator_object<T>(st.top(),
                                                     m_functions.find(st.top())->second));
                        st.pop();
                    }
                    break;
                }
                default:
                    break;
                }
                token_pos_curr++;
            }
            if(m_transition_table[table_pos_curr].Return)
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
                table_pos_curr = (size_t)m_transition_table[table_pos_curr].Jump;
        }
        else
        {
            if(m_transition_table[table_pos_curr].Error)
            {
                m_status = false;
                if(token_pos_curr < tokens.size())
                    m_error_string = string("Bad token `") + tokens[token_pos_curr] + string("`!");
                else
                    m_error_string = "Unexpected end of string!";
                return false;
            }
            else
            {
                table_pos_curr++;
            }
        }
    }

    while(m_status && !st.empty())
    {
        if(m_operators.find(st.top()[0]) == m_operators.end())
        {
            m_status = false;
            m_error_string = "Wrong expression!";
            break;
        }
        m_expression.push_back(evaluator_object<T>(st.top(),
                                     m_operators.find(st.top()[0])->second.second));
        st.pop();
    }

    return m_status;
}

#endif // EVALUATOR_PARSE_H

