#ifndef FUNC_TEMPLATES_H
#define FUNC_TEMPLATES_H

#include <typeinfo>
#include <complex>
#include <cstring>
#include <cstdlib>
#include "common.h"

namespace evaluator_internal_jit
{

// Test functions which is used if ABI is unknown
void EVALUATOR_JIT_CALL test_func_flt();
void EVALUATOR_JIT_CALL test_func_dbl();
void EVALUATOR_JIT_CALL test_func_cflt();
void EVALUATOR_JIT_CALL test_func_cdbl();

// Pre-compiled bytecode of function call
extern const char * code_func_flt;
extern const char * code_func_dbl;
extern const char * code_func_cflt;
extern const char * code_func_cdbl;


// Function calls generator for functions with 1 argument
template<typename T>
class jit_func_generator
{
private:

    const jit_func_generator & operator = (const jit_func_generator & other);
    jit_func_generator(const jit_func_generator & other);

protected:

    size_t code_len;
    char * raw_data;
    size_t offset_return;
    size_t offset_return_2;
    size_t offset_arg;
    size_t offset_func;
    bool status;

public:

    jit_func_generator(bool active)
    {
        status = active;
        raw_data = NULL;
        code_len = 0;
        offset_return = offset_return_2 = offset_arg = offset_func = 0;
        if(!active) return;
        const char * tc = NULL;
        if(typeid(T) == typeid(double))
            tc = code_func_dbl;
        else if(typeid(T) == typeid(float))
            tc = code_func_flt;
        else if(typeid(T) == typeid(std::complex<double>))
            tc = code_func_cdbl;
        else if(typeid(T) == typeid(std::complex<float>))
            tc = code_func_cflt;

        if(!tc)
        {
            if(typeid(T) == typeid(double))
            {
                void(EVALUATOR_JIT_CALL * func)() = test_func_dbl;
                size_t call_addr = (size_t)(& func);
                size_t code_addr = (size_t)(& tc);
                memcpy((void *)code_addr, (void *)call_addr, sizeof(void *));
            }
            else if(typeid(T) == typeid(float))
            {
                void(EVALUATOR_JIT_CALL * func)() = test_func_flt;
                size_t call_addr = (size_t)(& func);
                size_t code_addr = (size_t)(& tc);
                memcpy((void *)code_addr, (void *)call_addr, sizeof(void *));
            }
            else if(typeid(T) == typeid(std::complex<double>))
            {
                void(EVALUATOR_JIT_CALL * func)() = test_func_cdbl;
                size_t call_addr = (size_t)(& func);
                size_t code_addr = (size_t)(& tc);
                memcpy((void *)code_addr, (void *)call_addr, sizeof(void *));
            }
            else if(typeid(T) == typeid(std::complex<float>))
            {
                void(EVALUATOR_JIT_CALL * func)() = test_func_cflt;
                size_t call_addr = (size_t)(& func);
                size_t code_addr = (size_t)(& tc);
                memcpy((void *)code_addr, (void *)call_addr, sizeof(void *));
            }
        }

        if(!tc)
        {
            status = false;
            return;
        }
        while(tc[0] == '\xe9') // jump
        {
            size_t offset;
            memcpy(&offset, tc + 1, 4);
            tc += offset + 5;
        }
        const char * tcc = tc;
        while(* tcc != '\xc3') // ret
        {
            tcc++;
            code_len++;
        }
        raw_data = new char [code_len + 10 * sizeof(size_t)];
        memcpy(raw_data, tc, code_len);
        for(size_t i = 0; i < code_len; i++)
        {
            // return pointer - 0xC0FFEE03
            if(raw_data[i] == '\x03' && raw_data[i + 1] == '\xee' &&
                    raw_data[i + 2] == '\xff' && raw_data[i + 3] == '\xc0')
                offset_return = i;
            // second part of return pointer (optional)
            // complex<double> - 0xC0FFEE0B
            // complex<float>  - 0xC0FFEE07
            if((raw_data[i] == '\x0b' || raw_data[i] == '\x07') &&
                    raw_data[i + 1] == '\xee' &&  raw_data[i + 2] == '\xff' &&
                    raw_data[i + 3] == '\xc0')
                offset_return_2 = i;
            // arg pointer - 0xC0FFEE01
            if(raw_data[i] == '\x01' && raw_data[i + 1] == '\xee' &&
                    raw_data[i + 2] == '\xff' && raw_data[i + 3] == '\xc0')
                offset_arg = i;
            // function pointer - 0xDEADBEEF
            if(raw_data[i] == '\xef' && raw_data[i + 1] == '\xbe' &&
                    raw_data[i + 2] == '\xad' && raw_data[i + 3] == '\xde')
                offset_func = i;
        }
        if(offset_return == 0 || offset_arg == 0 || offset_func == 0)
            status = false;
    }

    ~jit_func_generator()
    {
        delete [] raw_data;
    }

    void call(char *& code_curr, T(* const func)(const T &), const T * arg, const T * ret) const
    {
        memcpy(code_curr, raw_data, code_len);
        memcpy(code_curr + offset_func, &func, sizeof(size_t));
        memcpy(code_curr + offset_arg, &arg, sizeof(size_t));
        memcpy(code_curr + offset_return, &ret, sizeof(size_t));
        if(offset_return_2)
        {
            const void * ret2 = (const void *)(((size_t)(ret)) + (sizeof(T) / 2));
            memcpy(code_curr + offset_return_2, &ret2, sizeof(size_t));
        }
        code_curr += code_len;
    }

    bool check() const
    {
        return status;
    }
};

}

#endif // FUNC_TEMPLATES_H

