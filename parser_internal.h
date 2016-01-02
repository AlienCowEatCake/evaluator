#if !defined PARSER_INTERNAL_H
#define PARSER_INTERNAL_H

#include <cstdlib>
#include <string>
#include <vector>

// Internal parser's classes & functions
namespace parser_internal
{
    using namespace std;

    // =============================================================================================

    // Auto-allocatable container for variables.
    template<typename T> class var_container
    {
    protected:
        T * val;
    public:
        inline const T & value() const
        {
            return * val;
        }
        inline T & value()
        {
            return * val;
        }
        inline const T * pointer() const
        {
            return val;
        }
        inline T * pointer()
        {
            return val;
        }
        var_container(const T & new_val = T())
        {
            val = new T(new_val);
        }
        var_container(const var_container & other)
        {
            val = new T(other.value());
        }
        const var_container & operator = (const var_container & other)
        {
            if(this != & other)
            {
                val = new T;
                * val = other.value();
            }
            return * this;
        }
        ~var_container()
        {
            delete val;
        }
    };

    // =============================================================================================

    // The universal parser object. It may be operator, function, variable or constant.
    template<typename T>
    class parser_object
    {
    protected:
        enum parser_object_type
        {
            PI_OBJ_OPERATOR,
            PI_OBJ_FUNCTION,
            PI_OBJ_VARIABLE,
            PI_OBJ_CONSTANT
        };
        parser_object_type type;
        string str_;
        T(* func)(const T &);
        T(* oper)(const T &, const T &);
        T value;
        const T * var_value;
        void init(parser_object_type n_type, const string & n_str, T(* n_func)(const T &),
                  T(* n_oper)(const T &, const T &), const T & n_value, const T * n_var_value)
        {
            type = n_type;
            str_ = n_str;
            value = n_value;
            var_value = n_var_value;
            func = n_func;
            oper = n_oper;
        }
    public:
        inline bool is_variable() const
        {
            return type == PI_OBJ_VARIABLE;
        }
        inline bool is_constant() const
        {
            return type == PI_OBJ_CONSTANT;
        }
        inline bool is_function() const
        {
            return type == PI_OBJ_FUNCTION;
        }
        inline bool is_operator() const
        {
            return type == PI_OBJ_OPERATOR;
        }
        inline const string & str() const
        {
            return str_;
        }
        inline const T * raw_value() const
        {
            return (type == PI_OBJ_VARIABLE ? var_value : (& value));
        }
        inline T(* raw_oper() const)(const T &, const T &)
        {
            return oper;
        }
        inline T(* raw_func() const)(const T &)
        {
            return func;
        }
        inline T eval() const
        {
            return (type == PI_OBJ_VARIABLE ? (* var_value) : value);
        }
        inline T eval(const T & arg) const
        {
            return func(arg);
        }
        inline T eval(const T & larg, const T & rarg) const
        {
            return oper(larg, rarg);
        }
        parser_object(const string & new_str, const T * new_var_value)
        {
            init(PI_OBJ_VARIABLE, new_str, NULL, NULL, T(), new_var_value);
        }
        parser_object(const string & new_str, const T & new_value)
        {
            init(PI_OBJ_CONSTANT, new_str, NULL, NULL, new_value, NULL);
        }
        parser_object(const string & new_str, T(* new_func)(const T &))
        {
            init(PI_OBJ_FUNCTION, new_str, new_func, NULL, T(), NULL);
        }
        parser_object(const string & new_str, T(* new_oper)(const T &, const T &))
        {
            init(PI_OBJ_OPERATOR, new_str, NULL, new_oper, T(), NULL);
        }
    };

    // =============================================================================================

    template<typename T, const size_t max_size = 16>
    class simple_stack
    {
    public:
        inline const T & top() const
        {
            return data[top_place];
        }
        inline void pop()
        {
            top_place--;
        }
        inline size_t size() const
        {
            return top_place;
        }
        inline void push(const T & val)
        {
            data[++top_place] = val;
        }
        simple_stack()
        {
            top_place = 0;
        }
    protected:
        size_t top_place;
        T data[max_size];
    };

    // =============================================================================================

    class parser_table_record
    {
    public:
        vector<string> Terminals;
        int Jump;
        bool Accept;
        bool Stack;
        bool Return;
        bool Error;
        inline void set_values(const string & T, int J, bool A, bool S, bool R, bool E)
        {
            Jump = J;
            Accept = A;
            Stack = S;
            Return = R;
            Error = E;

            size_t beg = 0, end = T.find_first_of(" \t\r\n");
            while(end != string::npos)
            {
                Terminals.push_back(T.substr(beg, end - beg));
                beg = end + 1;
                end = T.find_first_of(" \t\r\n", beg);
            }
            Terminals.push_back(T.substr(beg));
        }
    };

    // =============================================================================================
}


#endif // PARSER_INTERNAL_H

