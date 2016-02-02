#ifndef OPER_TEMPLATES_H
#define OPER_TEMPLATES_H

#include <typeinfo>
#include <complex>
#include <cstring>
#include <cstdlib>
#include "common.h"

namespace evaluator_internal_jit
{

// Test functions which is used if ABI is unknown
void EVALUATOR_JIT_CALL test_oper_flt();
void EVALUATOR_JIT_CALL test_oper_dbl();
void EVALUATOR_JIT_CALL test_oper_cflt();
void EVALUATOR_JIT_CALL test_oper_cdbl();

// Pre-compiled bytecode of function call
extern const char * code_oper_flt;
extern const char * code_oper_dbl;
extern const char * code_oper_cflt;
extern const char * code_oper_cdbl;


// Function calls generator for functions with 2 arguments
template<typename T>
class jit_oper_generator
{
private:

    const jit_oper_generator & operator = (const jit_oper_generator & other);
    jit_oper_generator(const jit_oper_generator & other);

protected:

    size_t code_len;
    char * raw_data;
    size_t offset_return;
    size_t offset_return_2;
    size_t offset_larg;
    size_t offset_rarg;
    size_t offset_func;
    bool status;

public:

    jit_oper_generator(bool active)
    {
        status = true;
        raw_data = NULL;
        code_len = 0;
        offset_return = offset_return_2 = offset_larg = offset_rarg = offset_func = 0;
        if(!active) return;
        const char * tc = NULL;
        if(typeid(T) == typeid(double))
            tc = code_oper_dbl;
        else if(typeid(T) == typeid(float))
            tc = code_oper_flt;
        else if(typeid(T) == typeid(std::complex<double>))
            tc = code_oper_cdbl;
        else if(typeid(T) == typeid(std::complex<float>))
            tc = code_oper_cflt;

        if(!tc)
        {
            if(typeid(T) == typeid(double))
            {
                void(EVALUATOR_JIT_CALL * func)() = test_oper_dbl;
                size_t call_addr = (size_t)(& func);
                size_t code_addr = (size_t)(& tc);
                memcpy((void *)code_addr, (void *)call_addr, sizeof(void *));
            }
            else if(typeid(T) == typeid(float))
            {
                void(EVALUATOR_JIT_CALL * func)() = test_oper_flt;
                size_t call_addr = (size_t)(& func);
                size_t code_addr = (size_t)(& tc);
                memcpy((void *)code_addr, (void *)call_addr, sizeof(void *));
            }
            else if(typeid(T) == typeid(std::complex<double>))
            {
                void(EVALUATOR_JIT_CALL * func)() = test_oper_cdbl;
                size_t call_addr = (size_t)(& func);
                size_t code_addr = (size_t)(& tc);
                memcpy((void *)code_addr, (void *)call_addr, sizeof(void *));
            }
            else if(typeid(T) == typeid(std::complex<float>))
            {
                void(EVALUATOR_JIT_CALL * func)() = test_oper_cflt;
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
            size_t offset = 0;
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
            // larg pointer - 0xC0FFEE01
            if(raw_data[i] == '\x01' && raw_data[i + 1] == '\xee' &&
                    raw_data[i + 2] == '\xff' && raw_data[i + 3] == '\xc0')
                offset_larg = i;
            // rarg pointer - 0xC0FFEE02
            if(raw_data[i] == '\x02' && raw_data[i + 1] == '\xee' &&
                    raw_data[i + 2] == '\xff' && raw_data[i + 3] == '\xc0')
                offset_rarg = i;
            // function pointer - 0xDEADBEEF
            if(raw_data[i] == '\xef' && raw_data[i + 1] == '\xbe' &&
                    raw_data[i + 2] == '\xad' && raw_data[i + 3] == '\xde')
                offset_func = i;
        }
        if(offset_return == 0 || offset_larg == 0 || offset_rarg == 0 || offset_func == 0)
            status = false;
    }

    ~jit_oper_generator()
    {
        delete [] raw_data;
    }

    void call(char *& code_curr, T(* const func)(const T &, const T &), const T * larg, const T * rarg, const T * ret) const
    {
        memcpy(code_curr, raw_data, code_len);
        memcpy(code_curr + offset_func, &func, sizeof(size_t));
        memcpy(code_curr + offset_larg, &larg, sizeof(size_t));
        memcpy(code_curr + offset_rarg, &rarg, sizeof(size_t));
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

#endif // OPER_TEMPLATES_H

