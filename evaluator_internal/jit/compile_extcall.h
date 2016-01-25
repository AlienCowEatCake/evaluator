#ifndef COMPILE_EXTCALL_H
#define COMPILE_EXTCALL_H

#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <typeinfo>
#include <complex>
#include <sstream>
#include "common.h"
#include "opcodes.h"
#include "func_templates.h"
#include "oper_templates.h"
#include "../../evaluator.h"

// Compile expression, all functions will be called from 'functions' and 'operators' containers
template<typename T>
bool evaluator<T>::compile_extcall()
{
#if !defined(EVALUATOR_JIT_DISABLE)
    using namespace std;
    using namespace evaluator_internal;
    using namespace evaluator_internal_jit;

    if(!is_parsed())
    {
        error_string = "Not parsed!";
        return false;
    }

    if(!jit_code || !jit_code_size)
    {
        jit_code_size = 128 * 1024; // 128 KiB
        jit_code = (char *)exec_alloc(jit_code_size);
        size_t call_addr = (size_t)(& jit_func);
        size_t code_addr = (size_t)(& jit_code);
        memcpy((void *)call_addr, (void *)code_addr, sizeof(void *));
    }
    memset(jit_code, '\xc3', jit_code_size);

    if(!jit_stack || !jit_stack_size)
    {
        jit_stack_size = 128 * 1024 / sizeof(T); // 128 KiB
        jit_stack = new T [jit_stack_size];
    }
    memset(jit_stack, 0, jit_stack_size);

    char * curr = jit_code;
    T * jit_stack_curr = jit_stack;

#if defined(EVALUATOR_JIT_X86) || defined(EVALUATOR_JIT_X64)

    jit_oper_generator<T> f2arg(true);
    jit_func_generator<T> f1arg(true);
    if(!f2arg.check() || !f1arg.check())
    {
        error_string = string("Unsupported calling convention for type `") +
                       typeid(T).name() + string("`!");
        return false;
    }

    if((typeid(T) == typeid(float) && sizeof(float) == 4) ||
       (typeid(T) == typeid(double) && sizeof(double) == 8))
    {
        for(typename vector<evaluator_object<T> >::const_iterator
            it = expression.begin(); it != expression.end(); ++it)
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
    else if((typeid(T) == typeid(complex<float>) && sizeof(float) == 4) ||
            (typeid(T) == typeid(complex<double>) && sizeof(double) == 8))
    {
        for(typename vector<evaluator_object<T> >::const_iterator
            it = expression.begin(); it != expression.end(); ++it)
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
        error_string = "Unsupported type `" + string(typeid(T).name()) + "`!";
        return false;
    }

    ret(curr);

#else
    (void)(curr);
    error_string = "Unsupported arch!";
    return false;
#endif

    if(jit_stack_curr != jit_stack)
    {
        stringstream sst;
        sst << "Stack size equal " << (size_t)(jit_stack_curr - jit_stack);
        error_string = sst.str();
        return false;
    }

    is_compiled = true;
    return true;
#else
    error_string = "JIT is disabled!";
    return false;
#endif
}

#endif // COMPILE_EXTCALL_H

