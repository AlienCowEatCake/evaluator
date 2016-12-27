#if !defined(EVALUATOR_OPCODES_H)
#define EVALUATOR_OPCODES_H

#include <complex>
#include <cassert>
#include "common.h"
#include "../type_detection.h"

// http://www.intel-assembler.it/portale/5/The-8087-Instruction-Set/A-one-line-description-of-x87-instructions.asp

namespace evaluator_internal_jit
{

// initialize 87
inline void finit(char *& code_curr)
{
    *(code_curr++) = '\xdb';
    *(code_curr++) = '\xe3';
}

inline void ret(char *& code_curr)
{
    *(code_curr++) = '\xc3';
}

// push, 0 := mem
template<typename T>
void fld_ptr(char *& code_curr, const T * ptr)
{
    using namespace evaluator_internal;
#if defined(EVALUATOR_JIT_X86)
    // fld    [dq]word ptr ds:[ptr]
    if(is_float<T>())
        *(code_curr++) = '\xd9';
    else if(is_double<T>())
        *(code_curr++) = '\xdd';
    else
        assert(false);
    *(code_curr++) = '\x05';
    const char * tmp_mem = reinterpret_cast<const char *>(ptr);
    memcpy(code_curr, & tmp_mem, sizeof(T*));
    code_curr += sizeof(T*);
#elif defined(EVALUATOR_JIT_X64)
    // mov    rdx, 0aaaaaaaaaaaaaaah
    *(code_curr++) = '\x48';
    *(code_curr++) = '\xba';
    const char * tmp_mem = reinterpret_cast<const char *>(ptr);
    memcpy(code_curr, & tmp_mem, sizeof(T*));
    code_curr += sizeof(T*);
    // fld    [dq]word ptr [rdx]
    if(is_float<T>())
        *(code_curr++) = '\xd9';
    else if(is_double<T>())
        *(code_curr++) = '\xdd';
    else
        assert(false);
    *(code_curr++) = '\x02';
#elif defined(EVALUATOR_JIT_X32)
    // mov    eax, 0xaaaaaaaa
    *(code_curr++) = '\xb8';
    const char * tmp_mem = reinterpret_cast<const char *>(ptr);
    memcpy(code_curr, & tmp_mem, sizeof(T*));
    code_curr += sizeof(T*);
    // fld    [dq]word ptr [eax]
    if(is_float<T>())
        *(code_curr++) = '\xd9';
    else if(is_double<T>())
        *(code_curr++) = '\xdd';
    else
        assert(false);
    *(code_curr++) = '\x00';
#endif
}

// mem := 0, pop
template<typename T>
void fstp_ptr(char *& code_curr, const T * ptr)
{
    using namespace evaluator_internal;
#if defined(EVALUATOR_JIT_X86)
    // fstp    [dq]word ptr ds:[ptr]
    if(is_float<T>())
        *(code_curr++) = '\xd9';
    else if(is_double<T>())
        *(code_curr++) = '\xdd';
    else
        assert(false);
    *(code_curr++) = '\x1d';
    const char * tmp_mem = reinterpret_cast<const char *>(ptr);
    memcpy(code_curr, & tmp_mem, sizeof(T*));
    code_curr += sizeof(T*);
#elif defined(EVALUATOR_JIT_X64)
    // mov    rdx, 0aaaaaaaaaaaaaaah
    *(code_curr++) = '\x48';
    *(code_curr++) = '\xba';
    const char * tmp_mem = reinterpret_cast<const char *>(ptr);
    memcpy(code_curr, & tmp_mem, sizeof(T*));
    code_curr += sizeof(T*);
    // fstp    [dq]word ptr [rdx]
    if(is_float<T>())
        *(code_curr++) = '\xd9';
    else if(is_double<T>())
        *(code_curr++) = '\xdd';
    else
        assert(false);
    *(code_curr++) = '\x1a';
#elif defined(EVALUATOR_JIT_X32)
    // mov    eax, 0xaaaaaaaa
    *(code_curr++) = '\xb8';
    const char * tmp_mem = reinterpret_cast<const char *>(ptr);
    memcpy(code_curr, & tmp_mem, sizeof(T*));
    code_curr += sizeof(T*);
    // fstp    [dq]word ptr [eax]
    if(is_float<T>())
        *(code_curr++) = '\xd9';
    else if(is_double<T>())
        *(code_curr++) = '\xdd';
    else
        assert(false);
    *(code_curr++) = '\x18';
#endif
}

// 1 := 1 + 0, pop
inline void fadd(char *& code_curr)
{
    *(code_curr++) = '\xde';
    *(code_curr++) = '\xc1';
}

// 1 := 1 - 0, pop
inline void fsub(char *& code_curr)
{
    *(code_curr++) = '\xde';
    *(code_curr++) = '\xe9';
}

// 1 := 0 - 1, pop
inline void fsubr(char *& code_curr)
{
    *(code_curr++) = '\xde';
    *(code_curr++) = '\xe1';
}

// 1 := 1 * 0, pop
inline void fmul(char *& code_curr)
{
    *(code_curr++) = '\xde';
    *(code_curr++) = '\xc9';
}

// 1 := 1 / 0, pop
inline void fdiv(char *& code_curr)
{
    *(code_curr++) = '\xde';
    *(code_curr++) = '\xf9';
}

// 1 := 0 / 1, pop
inline void fdivr(char *& code_curr)
{
    *(code_curr++) = '\xde';
    *(code_curr++) = '\xf1';
}

// 0 := 1 * log base 2.0 of 0, pop
inline void fyl2x(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xf1';
}

// 0 := (2.0 ** 0) - 1.0
inline void f2xm1(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xf0';
}

// push, 0 := 1.0
inline void fld1(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xe8';
}

// push, 0 := Pi
inline void fldpi(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xeb';
}

// push, 0 := +0.0
inline void fldz(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xee';
}

// exchange 0 and i
inline void fxch(char *& code_curr, int i = 1)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xc8' + static_cast<char>(i);
}

// 0 := 0 * 2.0 ** 1
inline void fscale(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xfd';
}

// 387 only: push, 1/0 := sine(old 0)
inline void fsin(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xfe';
}

// 387 only: push, 1/0 := cosine(old 0)
inline void fcos(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xff';
}

// 387 only: push, 1 := sine, 0 := cos(old 0)
inline void fsincos(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xfb';
}

// 0 := square root of 0
inline void fsqrt(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xfa';
}

// push, 1/0 := tan(old 0)
inline void fptan(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xf2';
}

// 0 := arctan(1/0), pop
inline void fpatan(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xf3';
}

// push, 0 := log base 2.0 of e
inline void fldl2e(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xea';
}

// push, 0 := log base 2.0 of 10.0
inline void fldl2t(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xe9';
}

// 0 := |0|
inline void fabs(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xe1';
}

// push, 0 := log base e of 2.0
inline void fldln2(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xed';
}

// 0 := round(0)
inline void frndint(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xfc';
}

// push, 0 := old i
inline void fldi(char *& code_curr, int i)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xc0' + static_cast<char>(i);
}

// i := 0, pop
inline void fstpi(char *& code_curr, int i)
{
    *(code_curr++) = '\xdd';
    *(code_curr++) = '\xd8' + static_cast<char>(i);
}

// 0 := -0
inline void fchs(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xe0';
}

// push, 1 := expo, 0 := sig
inline void fxtract(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xf4';
}

// compare 0 - 0.0
inline void ftst(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xe4';
}

// AX := status word
inline void fstsw_ax(char *& code_curr)
{
    *(code_curr++) = '\xdf';
    *(code_curr++) = '\xe0';
}

// Store AH into flags SF ZF xx AF xx PF xx CF
inline void sahf(char *& code_curr)
{
    *(code_curr++) = '\x9e';
}

// compare 0 - 1, pop
inline void fcomp(char *& code_curr)
{
    *(code_curr++) = '\xd8';
    *(code_curr++) = '\xd9';
}

// Set CF to 1
inline void stc(char *& code_curr)
{
    *(code_curr++) = '\xf9';
}

// 1-byte jump if ==
inline void jz(char *& code_curr, char * code_jump)
{
    if(code_jump)
    {
        const std::size_t diff = reinterpret_cast<std::size_t>(code_jump) - reinterpret_cast<std::size_t>(code_curr) - 2;
        *(code_curr++) = '\x74';
        memcpy(code_curr, & diff, 1);
        code_curr++;
    }
    else
    {
        memset(code_curr, '\x90', 2);
        code_curr += 2;
    }
}

// 1-byte jump if <
inline void ja(char *& code_curr, char * code_jump)
{
    if(code_jump)
    {
        const std::size_t diff = reinterpret_cast<std::size_t>(code_jump) - reinterpret_cast<std::size_t>(code_curr) - 2;
        *(code_curr++) = '\x77';
        memcpy(code_curr, & diff, 1);
        code_curr++;
    }
    else
    {
        memset(code_curr, '\x90', 2);
        code_curr += 2;
    }
}

// 1-byte jump if >
inline void jb(char *& code_curr, char * code_jump)
{
    if(code_jump)
    {
        const std::size_t diff = reinterpret_cast<std::size_t>(code_jump) - reinterpret_cast<std::size_t>(code_curr) - 2;
        *(code_curr++) = '\x72';
        memcpy(code_curr, & diff, 1);
        code_curr++;
    }
    else
    {
        memset(code_curr, '\x90', 2);
        code_curr += 2;
    }
}

// 1-byte jump if !=
inline void jnz(char *& code_curr, char * code_jump)
{
    if(code_jump)
    {
        const std::size_t diff = reinterpret_cast<std::size_t>(code_jump) - reinterpret_cast<std::size_t>(code_curr) - 2;
        *(code_curr++) = '\x75';
        memcpy(code_curr, & diff, 1);
        code_curr++;
    }
    else
    {
        memset(code_curr, '\x90', 2);
        code_curr += 2;
    }
}

// 1-byte jump
inline void jmp(char *& code_curr, char * code_jump)
{
    if(code_jump)
    {
        const std::size_t diff = reinterpret_cast<std::size_t>(code_jump) - reinterpret_cast<std::size_t>(code_curr) - 2;
        *(code_curr++) = '\xeb';
        memcpy(code_curr, & diff, 1);
        code_curr++;
    }
    else
    {
        memset(code_curr, '\x90', 2);
        code_curr += 2;
    }
}

// mov  bl,ah
inline void mov_bl_ah(char *& code_curr)
{
    *(code_curr++) = '\x88';
    *(code_curr++) = '\xe3';
}

// test bl,1
inline void test_bl_1(char *& code_curr)
{
    *(code_curr++) = '\xf6';
    *(code_curr++) = '\xc3';
    *(code_curr++) = '\x01';
}

// fprem
inline void fprem(char *& code_curr)
{
    *(code_curr++) = '\xd9';
    *(code_curr++) = '\xf8';
}

// push ebx(rbx)
inline void push_ebx(char *& code_curr)
{
    *(code_curr++) = '\x53';
}

// pop ebx(rbx)
inline void pop_ebx(char *& code_curr)
{
    *(code_curr++) = '\x5b';
}

// =============================================================================================

// st(0) = 2.0
inline void fld2(char *& code_curr)
{
    fld1(code_curr);
    fld1(code_curr);
    fadd(code_curr);
}

// Load real part of 'ptr'
template<typename T>
inline void fld_ptr_real(char *& code_curr, const std::complex<T> * ptr)
{
    const T * arr = reinterpret_cast<const T *>(ptr);
    fld_ptr(code_curr, &(arr[0]));
}

// Load imag part of 'ptr'
template<typename T>
inline void fld_ptr_imag(char *& code_curr, const std::complex<T> * ptr)
{
    const T * arr = reinterpret_cast<const T *>(ptr);
    fld_ptr(code_curr, &(arr[1]));
}

// Store to real part of 'ptr'
template<typename T>
inline void fstp_ptr_real(char *& code_curr, const std::complex<T> * ptr)
{
    const T * arr = reinterpret_cast<const T *>(ptr);
    fstp_ptr(code_curr, &(arr[0]));
}

// Store to imag part of 'ptr'
template<typename T>
inline void fstp_ptr_imag(char *& code_curr, const std::complex<T> * ptr)
{
    const T * arr = reinterpret_cast<const T *>(ptr);
    fstp_ptr(code_curr, &(arr[1]));
}

// Fake functions
template<typename T>
inline void fld_ptr_real(char *& code_curr, const T * ptr)
{
    (void)ptr;
    (void)code_curr;
    assert(false);
}

template<typename T>
inline void fld_ptr_imag(char *& code_curr, const T * ptr)
{
    (void)ptr;
    (void)code_curr;
    assert(false);
}

template<typename T>
inline void fstp_ptr_real(char *& code_curr, const T * ptr)
{
    (void)ptr;
    (void)code_curr;
    assert(false);
}

template<typename T>
inline void fstp_ptr_imag(char *& code_curr, const T * ptr)
{
    (void)ptr;
    (void)code_curr;
    assert(false);
}

} // namespace evaluator_internal_jit


#endif // EVALUATOR_OPCODES_H

