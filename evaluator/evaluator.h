#if !defined(EVALUATOR_H)
#define EVALUATOR_H

/*
// This is the module to parse, simplify and evaluate expressions with support of a JIT-compilation
//
// CC0 1.0 Universal License
// <http://creativecommons.org/publicdomain/zero/1.0/>
//
//
// Usage example:
//
// #include "evaluator/evaluator.h"
// ...
// evaluator<double> p;
// if(!p.parse("exp(-(0.5-x)*(0.5-x)-(0.5-z)*(0.5-z))"))
//     std::cerr << p.get_error() << std::endl;
// else if(!p.simplify()) // simplify is optional step
//     std::cerr << p.get_error() << std::endl;
// else
// {
//     if(!p.compile()) // compile is optional step
//        std::cerr << p.get_error() << std::endl;
//     p.set_var("x", 0.4);
//     p.set_var("z", 0.8);
//     double result;
//     if(!p.calculate(result))
//        std::cerr << p.get_error() << std::endl;
//     else
//         std::cout << result << std::endl;
// }
*/

#include "evaluator_operations.h"
#include "evaluator_internal/evaluator_object.h"
#include "evaluator_internal/var_container.h"
#include "evaluator_internal/transition_table.h"
#include "evaluator_internal/jit/common.h"
#include "evaluator_internal/jit/opcodes.h"
#include "evaluator_internal/jit/func_templates.h"
#include "evaluator_internal/jit/oper_templates.h"

#include <vector>
#include <map>
#include <string>
#include <utility>
#include <algorithm>

// Evaluator, main classs
template<typename T> class evaluator
{
public:

    // Function type in evaluator_object
    typedef T(* func_type)(const T &);
    // Operator type in evaluator_object
    typedef T(* oper_type)(const T &, const T &);

protected:

    // State transition table
    std::vector<evaluator_internal::transition_table_record> m_transition_table;
    // Expression, Reverse Polish notation
    std::vector<evaluator_internal::evaluator_object<T> > m_expression;
    // Container: [function name]->function pointer
    std::map<std::string, func_type> m_functions;
    // Container: [variable name]->pointer to var_container
    std::map<std::string, evaluator_internal::var_container<T> > m_variables;
    // Container: [constant name]->constant value
    std::map<std::string, T> m_constants;
    // Container: [operator name]->pair(priority, operator pointer)
    std::map<char, std::pair<unsigned short int, oper_type> > m_operators;
    // Current parsing status: true is good, false is bad
    bool m_status;
    // Error description if m_status == false
    std::string m_error_string;

    // Current compiling status: true is compiled, false is not compiled
    bool m_is_compiled;
#if !defined(EVALUATOR_JIT_DISABLE)
    // Executable memory for bytecode
    char * volatile m_jit_code;
    // Function pointer to same memory
    void(EVALUATOR_JIT_CALL * m_jit_func)();
    // Size of allocated executable memory
    std::size_t m_jit_code_size;
    // Memory for stack, used in compiled code
    T * volatile m_jit_stack;
    // Size of allocated memory for stack
    std::size_t m_jit_stack_size;
#endif

    // Return incorrect big number (uninitialized variable)
    template<typename U> T incorrect_number(const std::complex<U> &) const;
    template<typename U> T incorrect_number(const U &) const;
    // Check whether a number is incorrect (uninitialized variable)
    template<typename U> bool is_incorrect(const std::complex<U> & val) const;
    template<typename U> bool is_incorrect(const U & val) const;

    // Primary initialization
    void init();
    // Copying from another evaluator
    void copy_from_other(const evaluator & other);

public:

    // Constructors and destructor
    evaluator(const evaluator & other);
    evaluator();
    evaluator(const std::string & str);
    ~evaluator();
    // Copying from another evaluator
    const evaluator & operator = (const evaluator & other);

    // Get error description
    inline const std::string & get_error() const
    {
        return m_error_string;
    }

    // Get current parsing status
    inline bool is_parsed() const
    {
        return m_status;
    }

    // Set new value 'value' for variable with name 'name'
    inline void set_var(const std::string & name, const T & value)
    {
        m_variables[name].value() = value;
    }

    // Reset all variables
    void reset_vars();
    // Parse string 'str'
    bool parse(const std::string & str);
    // Simplify current expression
    bool simplify();
    // Calculate current expression and write result to 'result'
    bool calculate(T & result);

    // Get current compiling status
    inline bool is_compiled() const
    {
        return m_is_compiled;
    }

    // Compile expression, all functions will be inlined
    bool compile_inline();
    // Compile expression, all functions will be called from 'functions' and 'operators' containers
    bool compile_extcall();
    // Compile expression, default
    inline bool compile()
    {
        //return compile_extcall();
        return compile_inline();
    }

    // Print expression
    void debug_print() const;
};

#include "evaluator_internal/misc.h"
#include "evaluator_internal/parse.h"
#include "evaluator_internal/simplify.h"
#include "evaluator_internal/calculate.h"
#include "evaluator_internal/jit/compile_inline.h"
#include "evaluator_internal/jit/compile_extcall.h"

#endif // EVALUATOR_H

