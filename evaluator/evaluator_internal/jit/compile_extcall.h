#if !defined(EVALUATOR_COMPILE_EXTCALL_H)
#define EVALUATOR_COMPILE_EXTCALL_H

#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <complex>
#include <sstream>
#include "common.h"
#include "opcodes.h"
#include "func_templates.h"
#include "oper_templates.h"
#include "../type_detection.h"
#include "../../evaluator.h"

// Compile expression, all functions will be called from 'functions' and 'operators' containers
template<typename T>
bool evaluator<T>::compile_extcall()
{
#if !defined(EVALUATOR_JIT_DISABLE)
    using namespace evaluator_internal;
    using namespace evaluator_internal_jit;

    if(!is_parsed())
    {
        m_error_string = "Not parsed!";
        return false;
    }

    if(!m_jit_code || !m_jit_code_size)
    {
        m_jit_code_size = 128 * 1024; // 128 KiB
        m_jit_code = reinterpret_cast<char *>(exec_alloc(m_jit_code_size));
        std::size_t call_addr = reinterpret_cast<std::size_t>(& m_jit_func);
        std::size_t code_addr = reinterpret_cast<std::size_t>(& m_jit_code);
        memcpy(reinterpret_cast<void *>(call_addr), reinterpret_cast<void *>(code_addr), sizeof(void *));
    }
    memset(m_jit_code, '\xc3', m_jit_code_size);

    if(!m_jit_stack || !m_jit_stack_size)
    {
        m_jit_stack_size = 128 * 1024 / sizeof(T); // 128 KiB
        m_jit_stack = new T [m_jit_stack_size];
    }
    memset(m_jit_stack, 0, m_jit_stack_size);

    char * curr = m_jit_code;
    T * jit_stack_curr = m_jit_stack;

#if defined(EVALUATOR_JIT_X86) || defined(EVALUATOR_JIT_X64) || defined(EVALUATOR_JIT_X32)

    jit_oper_generator<T> f2arg(true);
    jit_func_generator<T> f1arg(true);
    if(!f2arg.check() || !f1arg.check())
    {
        m_error_string = std::string("Unsupported calling convention for type `") +
                         get_type_name<T>() + std::string("`!");
        return false;
    }

    if(is_float<T>() || is_double<T>())
    {
        for(typename std::vector<evaluator_object<T> >::const_iterator
            it = m_expression.begin(), it_end = m_expression.end(); it != it_end; ++it)
        {
            if(it->is_constant() || it->is_variable())
            {
                fld_ptr(curr, it->raw_value());
                fstp_ptr(curr, jit_stack_curr++);
            }
            else if(it->is_operator())
            {
                jit_stack_curr -= 2;
                f2arg.call(curr, it->raw_oper(), jit_stack_curr, jit_stack_curr + 1, jit_stack_curr);
                jit_stack_curr++;
            }
            else if(it->is_function())
            {
                jit_stack_curr--;
                f1arg.call(curr, it->raw_func(), jit_stack_curr, jit_stack_curr);
                jit_stack_curr++;
            }
        }

        jit_stack_curr--;
    }
    else if(is_complex_float<T>() || is_complex_double<T>())
    {
        for(typename std::vector<evaluator_object<T> >::const_iterator
            it = m_expression.begin(), it_end = m_expression.end(); it != it_end; ++it)
        {
            if(it->is_constant() || it->is_variable())
            {
                fld_ptr_real(curr, it->raw_value());
                fstp_ptr_real(curr, jit_stack_curr);
                fld_ptr_imag(curr, it->raw_value());
                fstp_ptr_imag(curr, jit_stack_curr++);
            }
            else if(it->is_operator())
            {
                jit_stack_curr -= 2;
                f2arg.call(curr, it->raw_oper(), jit_stack_curr, jit_stack_curr + 1, jit_stack_curr);
                jit_stack_curr++;
            }
            else if(it->is_function())
            {
                jit_stack_curr--;
                f1arg.call(curr, it->raw_func(), jit_stack_curr, jit_stack_curr);
                jit_stack_curr++;
            }
        }

        jit_stack_curr--;
    }
    else
    {
        m_error_string = "Unsupported type `" + get_type_name<T>() + "`!";
        return false;
    }

    ret(curr);

#else
    (void)(curr);
    m_error_string = "Unsupported arch!";
    return false;
#endif

    if(jit_stack_curr != m_jit_stack)
    {
        std::stringstream sst;
        sst << "Stack size equal " << static_cast<std::size_t>(jit_stack_curr - m_jit_stack);
        m_error_string = sst.str();
        return false;
    }

    m_is_compiled = true;
    return true;
#else
    m_error_string = "JIT is disabled!";
    return false;
#endif
}

#endif // EVALUATOR_COMPILE_EXTCALL_H

