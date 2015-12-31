#if !defined PARSER_OPCODES_H
#define PARSER_OPCODES_H

#include <cstdio>
#include <cstring>

#if (defined(_M_IX86 ) || defined(__X86__ ) || defined(__i386  ) || \
     defined(__IA32__) || defined(__I86__ ) || defined(__i386__) || \
     defined(__i486__) || defined(__i586__) || defined(__i686__))
#define PARSER_JIT_X86
#endif

#if (defined(_M_X64  ) || defined(__x86_64) || defined(__x86_64__) || \
     defined(_M_AMD64) || defined(__amd64 ) || defined(__amd64__ ))
#define PARSER_JIT_X64
#endif

#if !defined PARSER_JIT_X86 && !defined PARSER_JIT_X64
#define PARSER_JIT_DISABLE
#endif

#if !defined PARSER_JIT_DISABLE
#if defined _WIN32 || defined _WIN64
#if !defined NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/mman.h>
#endif
#if defined _MSC_VER
#define PARSER_JIT_CALL __cdecl
#elif defined __GNUC__
#define PARSER_JIT_CALL __attribute__((__cdecl__))
#else
#define PARSER_JIT_CALL
#endif
#endif

#if !defined PARSER_JIT_DISABLE

// Internal parser's functions for opcode generation
namespace parser_opcodes_generator
{
    inline void debug_asm_output(const char * fmt, ...)
    {
#if defined PARSER_ASM_DEBUG
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
#else
        (void)fmt;
#endif
    }

    // =============================================================================================

    // http://www.intel-assembler.it/portale/5/The-8087-Instruction-Set/A-one-line-description-of-x87-instructions.asp

    inline void finit(char *& code_curr)
    {
        *(code_curr++) = '\xdb';
        *(code_curr++) = '\xe3';
        debug_asm_output("finit\n");
    }

    inline void ret(char *& code_curr)
    {
        *(code_curr++) = '\xc3';
        debug_asm_output("ret\n");
    }

    template<typename T>
    void fld_ptr(char *& code_curr, const T * ptr)
    {
#if defined PARSER_JIT_X86
        // fld    [dq]word ptr ds:[ptr]
        if(typeid(T) == typeid(float))
            *(code_curr++) = '\xd9';
        else
            *(code_curr++) = '\xdd';
        *(code_curr++) = '\x05';
        const char * tmp_mem = reinterpret_cast<const char *>(ptr);
        memcpy(code_curr, & tmp_mem, sizeof(T*));
        code_curr += sizeof(T*);
        debug_asm_output("fld\t%cword ptr ds:[%xh]\n", (typeid(T) == typeid(float) ? 'd' : 'q'), (size_t)ptr);
#elif defined PARSER_JIT_X64
        // mov    rdx, 0aaaaaaaaaaaaaaah
        *(code_curr++) = '\x48';
        *(code_curr++) = '\xba';
        const char * tmp_mem = reinterpret_cast<const char *>(ptr);
        memcpy(code_curr, & tmp_mem, sizeof(T*));
        code_curr += sizeof(T*);
        debug_asm_output("mov\trdx, %llxh\n", (size_t)ptr);
        // fld    [dq]word ptr [rdx]
        if(typeid(T) == typeid(float))
            *(code_curr++) = '\xd9';
        else
            *(code_curr++) = '\xdd';
        *(code_curr++) = '\x02';
        debug_asm_output("fld\t%cword ptr [rdx]\n", (typeid(T) == typeid(float) ? 'd' : 'q'));
#endif
    }

    template<typename T>
    void fstp_ptr(char *& code_curr, const T * ptr)
    {
#if defined PARSER_JIT_X86
        // fstp    [dq]word ptr ds:[ptr]
        if(typeid(T) == typeid(float))
            *(code_curr++) = '\xd9';
        else
            *(code_curr++) = '\xdd';
        *(code_curr++) = '\x1d';
        const char * tmp_mem = reinterpret_cast<const char *>(ptr);
        memcpy(code_curr, & tmp_mem, sizeof(T*));
        code_curr += sizeof(T*);
        debug_asm_output("fstp\t%cword ptr ds:[%xh]\n", (typeid(T) == typeid(float) ? 'd' : 'q'), (size_t)ptr);
#elif defined PARSER_JIT_X64
        // mov    rdx, 0aaaaaaaaaaaaaaah
        *(code_curr++) = '\x48';
        *(code_curr++) = '\xba';
        const char * tmp_mem = reinterpret_cast<const char *>(ptr);
        memcpy(code_curr, & tmp_mem, sizeof(T*));
        code_curr += sizeof(T*);
        debug_asm_output("mov\trdx, %llxh\n", (size_t)ptr);
        // fstp    [dq]word ptr [rdx]
        if(typeid(T) == typeid(float))
            *(code_curr++) = '\xd9';
        else
            *(code_curr++) = '\xdd';
        *(code_curr++) = '\x1a';
        debug_asm_output("fstp\t%cword ptr [rdx]\n", (typeid(T) == typeid(float) ? 'd' : 'q'));
#endif
    }

    inline void fadd(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xc1';
        debug_asm_output("fadd\n");
    }

    inline void fsub(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xe9';
        debug_asm_output("fsub\n");
    }

    inline void fsubr(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xe1';
        debug_asm_output("fsubr\n");
    }

    inline void fmul(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xc9';
        debug_asm_output("fmul\n");
    }

    inline void fdiv(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xf9';
        debug_asm_output("fdiv\n");
    }

    inline void fdivr(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xf1';
        debug_asm_output("fdivr\n");
    }

    inline void fyl2x(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf1';
        debug_asm_output("fyl2x\n");
    }

    inline void f2xm1(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf0';
        debug_asm_output("f2xm1\n");
    }

    inline void fld1(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xe8';
        debug_asm_output("fld1\n");
    }

    inline void fldpi(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xeb';
        debug_asm_output("fldpi\n");
    }

    inline void fldz(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xee';
        debug_asm_output("fldz\n");
    }

    inline void fxch(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xc9';
        debug_asm_output("fxch\n");
    }

    inline void fxch(char *& code_curr, int i)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xc8' + i;
        debug_asm_output("fxch\t%d\n", i);
    }

    inline void fscale(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfd';
        debug_asm_output("fscale\n");
    }

    inline void fsin(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfe';
        debug_asm_output("fsin\n");
    }

    inline void fcos(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xff';
        debug_asm_output("fcos\n");
    }

    inline void fsincos(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfb';
        debug_asm_output("fsincos\n");
    }

    inline void fsqrt(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfa';
        debug_asm_output("fsqrt\n");
    }

    inline void fptan(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf2';
        debug_asm_output("fptan\n");
    }

    inline void fpatan(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf3';
        debug_asm_output("fpatan\n");
    }

    inline void fldl2e(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xea';
        debug_asm_output("fldl2e\n");
    }

    inline void fldl2t(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xe9';
        debug_asm_output("fldl2t\n");
    }

    inline void fabs(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xe1';
        debug_asm_output("fabs\n");
    }

    inline void fldln2(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xed';
        debug_asm_output("fldln2\n");
    }

    inline void frndint(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfc';
        debug_asm_output("frndint\n");
    }

    inline void fldi(char *& code_curr, int i)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xc0' + i;
        debug_asm_output("fldi\t%d\n", i);
    }

    inline void fchs(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xe0';
        debug_asm_output("fchs\n");
    }

    // =============================================================================================

    template<typename T>
    inline void fld_ptr_real(char *& code_curr, const std::complex<T> * ptr)
    {
        const T * arr = reinterpret_cast<const T *>(ptr);
        fld_ptr(code_curr, &(arr[0]));
    }

    template<typename T>
    inline void fld_ptr_imag(char *& code_curr, const std::complex<T> * ptr)
    {
        const T * arr = reinterpret_cast<const T *>(ptr);
        fld_ptr(code_curr, &(arr[1]));
    }

    template<typename T>
    inline void fstp_ptr_real(char *& code_curr, const std::complex<T> * ptr)
    {
        const T * arr = reinterpret_cast<const T *>(ptr);
        fstp_ptr(code_curr, &(arr[0]));
    }

    template<typename T>
    inline void fstp_ptr_imag(char *& code_curr, const std::complex<T> * ptr)
    {
        const T * arr = reinterpret_cast<const T *>(ptr);
        fstp_ptr(code_curr, &(arr[1]));
    }

    template<typename T>
    inline void fld_ptr_real(char *& code_curr, const T * ptr)
    {
        (void)code_curr;
        (void)ptr;
    }

    template<typename T>
    inline void fld_ptr_imag(char *& code_curr, const T * ptr)
    {
        (void)code_curr;
        (void)ptr;
    }

    template<typename T>
    inline void fstp_ptr_real(char *& code_curr, const T * ptr)
    {
        (void)code_curr;
        (void)ptr;
    }

    template<typename T>
    inline void fstp_ptr_imag(char *& code_curr, const T * ptr)
    {
        (void)code_curr;
        (void)ptr;
    }
}

#endif // !defined PARSER_JIT_DISABLE

#endif // PARSER_OPCODES_H

