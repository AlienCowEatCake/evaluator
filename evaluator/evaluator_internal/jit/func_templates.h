#if !defined(EVALUATOR_FUNC_TEMPLATES_H)
#define EVALUATOR_FUNC_TEMPLATES_H

#include <complex>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include "common.h"
#include "../type_detection.h"

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

    std::size_t m_code_len;
    char * m_raw_data;
    std::size_t m_offset_return;
    std::size_t m_offset_return_2;
    std::size_t m_offset_arg;
    std::size_t m_offset_func;
    bool m_status;

public:

    jit_func_generator(bool active)
    {
        using namespace evaluator_internal;
        m_status = active;
        m_raw_data = NULL;
        m_code_len = 0;
        m_offset_return = m_offset_return_2 = m_offset_arg = m_offset_func = 0;
        if(!active) return;
        const char * tc = NULL;
        if(is_double<T>())
            tc = code_func_dbl;
        else if(is_float<T>())
            tc = code_func_flt;
        else if(is_complex_double<T>())
            tc = code_func_cdbl;
        else if(is_complex_float<T>())
            tc = code_func_cflt;
        else
            assert(false);

        if(!tc)
        {
            void(EVALUATOR_JIT_CALL * func)() = NULL;
            if(is_double<T>())
                func = test_func_dbl;
            else if(is_float<T>())
                func = test_func_flt;
            else if(is_complex_double<T>())
                func = test_func_cdbl;
            else if(is_complex_float<T>())
                func = test_func_cflt;
            assert(func);
            if(func)
            {
                std::size_t call_addr = reinterpret_cast<std::size_t>(& func);
                std::size_t code_addr = reinterpret_cast<std::size_t>(& tc);
                memcpy(reinterpret_cast<void *>(code_addr), reinterpret_cast<void *>(call_addr), sizeof(void *));
            }
        }

        if(!tc)
        {
            m_status = false;
            return;
        }
        while(tc[0] == '\xe9') // jump
        {
            std::size_t offset = 0;
            memcpy(&offset, tc + 1, 4);
            tc += offset + 5;
        }
        const char * tcc = tc;
        while(* tcc != '\xc3') // ret
        {
            tcc++;
            m_code_len++;
        }
        m_raw_data = new char [m_code_len + 10 * sizeof(std::size_t)];
        memcpy(m_raw_data, tc, m_code_len);
        for(std::size_t i = 0; i < m_code_len; i++)
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
            // arg pointer - 0xC0FFEE01
            if(m_raw_data[i] == '\x01' && m_raw_data[i + 1] == '\xee' &&
                    m_raw_data[i + 2] == '\xff' && m_raw_data[i + 3] == '\xc0')
                m_offset_arg = i;
            // function pointer - 0xDEADBEEF
            if(m_raw_data[i] == '\xef' && m_raw_data[i + 1] == '\xbe' &&
                    m_raw_data[i + 2] == '\xad' && m_raw_data[i + 3] == '\xde')
                m_offset_func = i;
        }
        if(m_offset_return == 0 || m_offset_arg == 0 || m_offset_func == 0)
            m_status = false;
    }

    ~jit_func_generator()
    {
        delete [] m_raw_data;
    }

    void call(char *& code_curr, T(* const func)(const T &), const T * arg, const T * ret) const
    {
        memcpy(code_curr, m_raw_data, m_code_len);
        memcpy(code_curr + m_offset_func, &func, sizeof(std::size_t));
        memcpy(code_curr + m_offset_arg, &arg, sizeof(std::size_t));
        memcpy(code_curr + m_offset_return, &ret, sizeof(std::size_t));
        if(m_offset_return_2)
        {
            const void * ret2 = reinterpret_cast<const void *>(reinterpret_cast<std::size_t>(ret) + sizeof(T) / 2);
            memcpy(code_curr + m_offset_return_2, &ret2, sizeof(std::size_t));
        }
        code_curr += m_code_len;
    }

    bool check() const
    {
        return m_status;
    }
};

} // namespace evaluator_internal_jit

#endif // EVALUATOR_FUNC_TEMPLATES_H

