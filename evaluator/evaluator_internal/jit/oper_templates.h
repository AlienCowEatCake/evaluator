#ifndef EVALUATOR_OPER_TEMPLATES_H
#define EVALUATOR_OPER_TEMPLATES_H

#include <complex>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include "common.h"
#include "../type_detection.h"

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

    size_t m_code_len;
    char * m_raw_data;
    size_t m_offset_return;
    size_t m_offset_return_2;
    size_t m_offset_larg;
    size_t m_offset_rarg;
    size_t m_offset_func;
    bool m_status;

public:

    jit_oper_generator(bool active)
    {
        using namespace evaluator_internal;
        T * type_test = NULL;
        m_status = true;
        m_raw_data = NULL;
        m_code_len = 0;
        m_offset_return = m_offset_return_2 = m_offset_larg = m_offset_rarg = m_offset_func = 0;
        if(!active) return;
        const char * tc = NULL;
        if(is_double(type_test))
            tc = code_oper_dbl;
        else if(is_float(type_test))
            tc = code_oper_flt;
        else if(is_complex_double(type_test))
            tc = code_oper_cdbl;
        else if(is_complex_float(type_test))
            tc = code_oper_cflt;
        else
            assert(false);

        if(!tc)
        {
            if(is_double(type_test))
            {
                void(EVALUATOR_JIT_CALL * func)() = test_oper_dbl;
                size_t call_addr = (size_t)(& func);
                size_t code_addr = (size_t)(& tc);
                memcpy((void *)code_addr, (void *)call_addr, sizeof(void *));
            }
            else if(is_float(type_test))
            {
                void(EVALUATOR_JIT_CALL * func)() = test_oper_flt;
                size_t call_addr = (size_t)(& func);
                size_t code_addr = (size_t)(& tc);
                memcpy((void *)code_addr, (void *)call_addr, sizeof(void *));
            }
            else if(is_complex_double(type_test))
            {
                void(EVALUATOR_JIT_CALL * func)() = test_oper_cdbl;
                size_t call_addr = (size_t)(& func);
                size_t code_addr = (size_t)(& tc);
                memcpy((void *)code_addr, (void *)call_addr, sizeof(void *));
            }
            else if(is_complex_float(type_test))
            {
                void(EVALUATOR_JIT_CALL * func)() = test_oper_cflt;
                size_t call_addr = (size_t)(& func);
                size_t code_addr = (size_t)(& tc);
                memcpy((void *)code_addr, (void *)call_addr, sizeof(void *));
            }
            else
                assert(false);
        }

        if(!tc)
        {
            m_status = false;
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
            m_code_len++;
        }
        m_raw_data = new char [m_code_len + 10 * sizeof(size_t)];
        memcpy(m_raw_data, tc, m_code_len);
        for(size_t i = 0; i < m_code_len; i++)
        {
            // return pointer - 0xC0FFEE03
            if(m_raw_data[i] == '\x03' && m_raw_data[i + 1] == '\xee' &&
                    m_raw_data[i + 2] == '\xff' && m_raw_data[i + 3] == '\xc0')
                m_offset_return = i;
            // second part of return pointer (optional)
            // complex<double> - 0xC0FFEE0B
            // complex<float>  - 0xC0FFEE07
            if((m_raw_data[i] == '\x0b' || m_raw_data[i] == '\x07') &&
                    m_raw_data[i + 1] == '\xee' &&  m_raw_data[i + 2] == '\xff' &&
                    m_raw_data[i + 3] == '\xc0')
                m_offset_return_2 = i;
            // larg pointer - 0xC0FFEE01
            if(m_raw_data[i] == '\x01' && m_raw_data[i + 1] == '\xee' &&
                    m_raw_data[i + 2] == '\xff' && m_raw_data[i + 3] == '\xc0')
                m_offset_larg = i;
            // rarg pointer - 0xC0FFEE02
            if(m_raw_data[i] == '\x02' && m_raw_data[i + 1] == '\xee' &&
                    m_raw_data[i + 2] == '\xff' && m_raw_data[i + 3] == '\xc0')
                m_offset_rarg = i;
            // function pointer - 0xDEADBEEF
            if(m_raw_data[i] == '\xef' && m_raw_data[i + 1] == '\xbe' &&
                    m_raw_data[i + 2] == '\xad' && m_raw_data[i + 3] == '\xde')
                m_offset_func = i;
        }
        if(m_offset_return == 0 || m_offset_larg == 0 || m_offset_rarg == 0 || m_offset_func == 0)
            m_status = false;
    }

    ~jit_oper_generator()
    {
        delete [] m_raw_data;
    }

    void call(char *& code_curr, T(* const func)(const T &, const T &), const T * larg, const T * rarg, const T * ret) const
    {
        memcpy(code_curr, m_raw_data, m_code_len);
        memcpy(code_curr + m_offset_func, &func, sizeof(size_t));
        memcpy(code_curr + m_offset_larg, &larg, sizeof(size_t));
        memcpy(code_curr + m_offset_rarg, &rarg, sizeof(size_t));
        memcpy(code_curr + m_offset_return, &ret, sizeof(size_t));
        if(m_offset_return_2)
        {
            const void * ret2 = (const void *)(((size_t)(ret)) + (sizeof(T) / 2));
            memcpy(code_curr + m_offset_return_2, &ret2, sizeof(size_t));
        }
        code_curr += m_code_len;
    }

    bool check() const
    {
        return m_status;
    }
};

} // namespace evaluator_internal_jit

#endif // EVALUATOR_OPER_TEMPLATES_H

