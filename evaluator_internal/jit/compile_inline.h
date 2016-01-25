#ifndef COMPILE_INLINE_H
#define COMPILE_INLINE_H

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
#include "real_templates.h"
#include "complex_templates.h"
#include "../../evaluator.h"

// Compile expression, all functions will be inlined
template<typename T>
bool evaluator<T>::compile_inline()
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

    if((typeid(T) == typeid(float) && sizeof(float) == 4) ||
       (typeid(T) == typeid(double) && sizeof(double) == 8))
    {
        char * last_push_pos = NULL;
        T * last_push_val = NULL;
        for(typename vector<evaluator_object<T> >::const_iterator
            it = expression.begin(); it != expression.end(); ++it)
        {
            if(it->is_constant() || it->is_variable())
            {
                fld_ptr(curr, it->raw_value());
                last_push_pos = curr;
                last_push_val = jit_stack_curr;
                fstp_ptr(curr, jit_stack_curr++);
            }
            else if(it->is_operator())
            {
                jit_stack_curr -= 2;
                if(last_push_val == jit_stack_curr + 1)
                {
                    curr = last_push_pos;
                    fld_ptr(curr, jit_stack_curr);
                    fxch(curr);
                }
                else
                {
                    fld_ptr(curr, jit_stack_curr++);
                    fld_ptr(curr, jit_stack_curr--);
                }

                string op = it->str();
                if     (op[0] == '+')
                    fadd(curr);
                else if(op[0] == '-')
                    fsub(curr);
                else if(op[0] == '*')
                    fmul(curr);
                else if(op[0] == '/')
                    fdiv(curr);
                else if(op[0] == '^')
                {
                    fxch(curr);
                    real_pow(curr);
                }
                else
                {
                    error_string = "Unsupported operator " + it->str();
                    return false;
                }

                last_push_pos = curr;
                last_push_val = jit_stack_curr;
                fstp_ptr(curr, jit_stack_curr++);
            }
            else if(it->is_function())
            {
                jit_stack_curr--;
                if(last_push_val == jit_stack_curr)
                    curr = last_push_pos;
                else
                    fld_ptr(curr, jit_stack_curr);

                string fu = it->str();
                if     (fu == "sin")
                    fsin(curr);
                else if(fu == "cos")
                    fcos(curr);
                else if(fu == "sqrt")
                    fsqrt(curr);
                else if(fu == "tan")
                {
                    fptan(curr);
                    fdiv(curr);
                }
                else if(fu == "atan")
                {
                    fld1(curr);
                    fpatan(curr);
                }
                else if(fu == "asin")
                    real_asin(curr);
                else if(fu == "acos")
                    real_acos(curr);
                else if(fu == "log")
                    real_log(curr);
                else if(fu == "log2")
                    real_log2(curr);
                else if(fu == "log10")
                    real_log10(curr);
                else if(fu == "abs")
                    fabs(curr);
                else if(fu == "exp")
                    real_exp(curr);
                else if(fu == "sinh")
                    real_sinh(curr);
                else if(fu == "cosh")
                    real_cosh(curr);
                else if(fu == "tanh")
                    real_tanh(curr);
                else if(fu == "asinh")
                    real_asinh(curr);
                else if(fu == "acosh")
                    real_acosh(curr);
                else if(fu == "atanh")
                    real_atanh(curr);
                else if(fu == "imag")
                {
                    fldz(curr);
                    fmul(curr);
                }
                else if(fu == "arg")
                    real_arg(curr);
                else if(fu != "real" && fu != "conj")
                {
                    error_string = "Unsupported function " + it->str();
                    return false;
                }
                last_push_pos = curr;
                last_push_val = jit_stack_curr;
                fstp_ptr(curr, jit_stack_curr++);
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
                string op = it->str();
                jit_stack_curr -= 2;
                if(op[0] == '+')
                    complex_add(curr, jit_stack_curr, jit_stack_curr + 1, jit_stack_curr);
                else if(op[0] == '-')
                    complex_sub(curr, jit_stack_curr, jit_stack_curr + 1, jit_stack_curr);
                else if(op[0] == '*')
                    complex_mul(curr, jit_stack_curr, jit_stack_curr + 1, jit_stack_curr);
                else if(op[0] == '/')
                    complex_div(curr, jit_stack_curr, jit_stack_curr + 1, jit_stack_curr);
                else if(op[0] == '^')
                    complex_pow(curr, jit_stack_curr, jit_stack_curr + 1, jit_stack_curr, jit_stack_curr + 2);
                else
                {
                    error_string = "Unsupported operator " + it->str();
                    return false;
                }
                jit_stack_curr++;
            }
            else if(it->is_function())
            {
                string fu = it->str();
                jit_stack_curr--;
                if(fu == "real")
                    complex_real(curr, jit_stack_curr);
                else if(fu == "imag")
                    complex_imag(curr, jit_stack_curr);
                else if(fu == "conj")
                    complex_conj(curr, jit_stack_curr);
                else if(fu == "arg")
                    complex_arg(curr, jit_stack_curr, jit_stack_curr);
                else if(fu == "abs")
                    complex_abs(curr, jit_stack_curr, jit_stack_curr);
                else if(fu == "exp")
                    complex_exp(curr, jit_stack_curr, jit_stack_curr);
                else if(fu == "sin")
                    complex_sin(curr, jit_stack_curr, jit_stack_curr, jit_stack_curr + 1);
                else if(fu == "cos")
                    complex_cos(curr, jit_stack_curr, jit_stack_curr, jit_stack_curr + 1);
                else if(fu == "tan")
                    complex_tan(curr, jit_stack_curr, jit_stack_curr, jit_stack_curr + 1);
                else if(fu == "log")
                    complex_log(curr, jit_stack_curr, jit_stack_curr);
                else if(fu == "log2")
                    complex_log2(curr, jit_stack_curr, jit_stack_curr);
                else if(fu == "log10")
                    complex_log10(curr, jit_stack_curr, jit_stack_curr);
                else if(fu == "sqrt")
                    complex_sqrt(curr, jit_stack_curr, jit_stack_curr);
                else if(fu == "asin")
                    complex_asin(curr, jit_stack_curr, jit_stack_curr, jit_stack_curr + 1);
                else if(fu == "acos")
                    complex_acos(curr, jit_stack_curr, jit_stack_curr, jit_stack_curr + 1);
                else if(fu == "atan")
                    complex_atan(curr, jit_stack_curr, jit_stack_curr, jit_stack_curr + 1);
                else if(fu == "sinh")
                    complex_sinh(curr, jit_stack_curr, jit_stack_curr, jit_stack_curr + 1);
                else if(fu == "cosh")
                    complex_cosh(curr, jit_stack_curr, jit_stack_curr, jit_stack_curr + 1);
                else if(fu == "tanh")
                    complex_tanh(curr, jit_stack_curr, jit_stack_curr, jit_stack_curr + 1);
                else if(fu == "asinh")
                    complex_asinh(curr, jit_stack_curr, jit_stack_curr, jit_stack_curr + 1);
                else if(fu == "acosh")
                    complex_acosh(curr, jit_stack_curr, jit_stack_curr, jit_stack_curr + 1);
                else if(fu == "atanh")
                    complex_atanh(curr, jit_stack_curr, jit_stack_curr, jit_stack_curr + 1);
                else
                {
                    error_string = "Unsupported function " + it->str();
                    return false;
                }
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

#endif // COMPILE_INLINE_H

