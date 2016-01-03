#if !defined PARSER_COMPILER_EXTCALL_H
#define PARSER_COMPILER_EXTCALL_H

#include "parser.h"

template<typename T>
bool parser<T>::compile_extcall()
{
#if !defined PARSER_JIT_DISABLE
    using namespace std;
    using namespace parser_internal;
    using namespace parser_opcodes;
    using namespace parser_templates;

    if(!is_parsed())
    {
        error_string = "Not parsed!";
        return false;
    }

    if(!jit_code || !jit_code_size)
    {
        jit_code_size = 128 * 1024; // 128 KiB
#if defined _WIN32 || defined _WIN64
        jit_code = (char *)malloc(jit_code_size);
        DWORD tmp;
        VirtualProtect(jit_code, jit_code_size, PAGE_EXECUTE_READWRITE, &tmp);
#else
        int prot = PROT_READ | PROT_WRITE | PROT_EXEC;
        int flags = MAP_PRIVATE | MAP_ANONYMOUS;
        jit_code = (char *)mmap(NULL, jit_code_size, prot, flags, -1, 0);
#endif
    }
    memset(jit_code, 0, jit_code_size);

    if(!jit_stack || !jit_stack_size)
    {
        jit_stack_size = 128 * 1024 / sizeof(T); // 128 KiB
        jit_stack = new T [jit_stack_size];
    }

    memset(jit_code, '\xc3', jit_code_size);
    memset(jit_stack, 0, jit_stack_size);

    char * curr = jit_code;
    T * jit_stack_curr = jit_stack;

    functions_2arg_generator<T> f2arg;
    functions_1arg_generator<T> f1arg;
    if(!f2arg.check() || !f1arg.check())
    {
        error_string = string("Unsupported calling convention for type `") +
                       typeid(T).name() + string("`!");
        return false;
    }

#if defined PARSER_JIT_X86 || defined PARSER_JIT_X64

    if((typeid(T) == typeid(float) && sizeof(float) == 4) ||
       (typeid(T) == typeid(double) && sizeof(double) == 8))
    {
        for(typename vector<parser_object<T> >::const_iterator
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
        for(typename vector<parser_object<T> >::const_iterator
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

#endif // PARSER_COMPILER_EXTCALL_H

