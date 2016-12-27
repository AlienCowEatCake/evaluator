#if !defined(EVALUATOR_OBJECT_H)
#define EVALUATOR_OBJECT_H

#include <string>

namespace evaluator_internal
{

// The universal evaluator object. It may be operator, function, variable or constant.
template<typename T> class evaluator_object
{
private:

    // Allowed object types
    enum obj_type
    {
        OBJ_OPERATOR,
        OBJ_FUNCTION,
        OBJ_VARIABLE,
        OBJ_CONSTANT
    };
    // Type of object
    obj_type m_type;

    // Pointer to function  (for function type)
    typedef T(* func_type)(const T &);
    func_type m_func;

    // Pointer to operator (for operator type)
    typedef T(* oper_type)(const T &, const T &);
    oper_type m_oper;

    // Value of constant (for constant type)
    T m_const_value;

    // Pointer to variable (for variable type)
    const T * m_var_value;

    // String representation of object (for any type)
    std::string m_str;

    // Initialize all members
    void init(obj_type type, const std::string & str, func_type func,
              oper_type oper, const T & value, const T * var_value)
    {
        m_type = type;
        m_str = str;
        m_const_value = value;
        m_var_value = var_value;
        m_func = func;
        m_oper = oper;
    }

public:

    // Object is variable?
    inline bool is_variable() const
    {
        return m_type == OBJ_VARIABLE;
    }

    // Object is constant?
    inline bool is_constant() const
    {
        return m_type == OBJ_CONSTANT;
    }

    // Object is function?
    inline bool is_function() const
    {
        return m_type == OBJ_FUNCTION;
    }

    // Object is operator?
    inline bool is_operator() const
    {
        return m_type == OBJ_OPERATOR;
    }

    // Get string representation of object
    inline const std::string & str() const
    {
        return m_str;
    }

    // Get pointer to variable or constant
    inline const T * raw_value() const
    {
        return (m_type == OBJ_VARIABLE ? m_var_value : (& m_const_value));
    }

    // Get pointer to operator
    inline oper_type raw_oper() const
    {
        return m_oper;
    }

    // Get pointer to function
    inline func_type raw_func() const
    {
        return m_func;
    }

    // Get value of variable or constant
    inline T eval() const
    {
        return (m_type == OBJ_VARIABLE ? (* m_var_value) : m_const_value);
    }

    // Calc function with argument 'arg'
    inline T eval(const T & arg) const
    {
        return m_func(arg);
    }

    // Calc oparator with arguments 'larg' and 'rarg'
    inline T eval(const T & larg, const T & rarg) const
    {
        return m_oper(larg, rarg);
    }

    // Construct variable object
    evaluator_object(const std::string & new_str, const T * var_val)
    {
        init(OBJ_VARIABLE, new_str, NULL, NULL, T(), var_val);
    }

    // Construct constant object
    evaluator_object(const std::string & new_str, const T & const_val)
    {
        init(OBJ_CONSTANT, new_str, NULL, NULL, const_val, NULL);
    }

    // Construct function object
    evaluator_object(const std::string & new_str, func_type func)
    {
        init(OBJ_FUNCTION, new_str, func, NULL, T(), NULL);
    }

    // Construct operator object
    evaluator_object(const std::string & new_str, oper_type oper)
    {
        init(OBJ_OPERATOR, new_str, NULL, oper, T(), NULL);
    }
};

} // namespace evaluator_internal

#endif // EVALUATOR_OBJECT_H

