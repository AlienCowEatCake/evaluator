#ifndef EVALUATOR_H
#define EVALUATOR_H

/*
// Usage example:
//
// #include "evaluator.h"
// ...
// evaluator<double> p;
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
    std::vector<evaluator_internal::transition_table_record> transition_table;
    // Expression, Reverse Polish notation
    std::vector<evaluator_internal::evaluator_object<T> > expression;
    // Container: [function name]->function pointer
    std::map<std::string, func_type> functions;
    // Container: [variable name]->pointer to var_container
    std::map<std::string, evaluator_internal::var_container<T> > variables;
    // Container: [constant name]->constant value
    std::map<std::string, T> constants;
    // Container: [operator name]->pair(priority, operator pointer)
    std::map<char, std::pair<unsigned short int, oper_type> > operators;
    // Current parsing status: true is good, false is bad
    bool status;
    // Error description if status == false
    std::string error_string;

#if !defined(EVALUATOR_JIT_DISABLE)
    // Current conpiling status: true is compiled, false is not compiled
    bool is_compiled;
    // Executable memory for bytecode
    char * volatile jit_code;
    // Function pointer to same memory
    void(EVALUATOR_JIT_CALL * jit_func)();
    // Size of allocated executable memory
    size_t jit_code_size;
    // Memory for stack, used in compiled code
    T * volatile jit_stack;
    // Size of allocated memory for stack
    size_t jit_stack_size;
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
        return error_string;
    }

    // Get current parsing status
    inline bool is_parsed() const
    {
        return status;
    }

    // Set new value 'value' for variable with name 'name'
    inline void set_var(const std::string & name, const T & value)
    {
        variables[name].value() = value;
    }

    // Reset all variables
    void reset_vars();
    // Parse string 'str'
    bool parse(const std::string & str);
    // Simplify current expression
    bool simplify();
    // Calculate current expression and write result to 'result'
    bool calculate(T & result);

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

