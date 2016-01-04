#if !defined PARSER_OPCODES_H
#define PARSER_OPCODES_H

#include <cstdio>
#include <cstring>
#include <cstdarg>

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
#elif defined __GNUC__ && defined PARSER_JIT_X86
#define PARSER_JIT_CALL __attribute__((__cdecl__))
#else
#define PARSER_JIT_CALL
#endif
#endif

#if !defined PARSER_JIT_DISABLE
// System V ABI
#if (defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))) && \
    !(defined (__CYGWIN__) || defined (__MINGW32__)) && \
    (defined (PARSER_JIT_X86) || defined(PARSER_JIT_X64))
#define PARSER_JIT_SYSV_ABI
#endif
// MS Compiler ABI
#if (defined (_WIN32) || defined (_WIN64)) && \
    !(defined (__CYGWIN__) || defined (__MINGW32__)) && \
    (defined (PARSER_JIT_X86) || defined(PARSER_JIT_X64))
#define PARSER_JIT_MSVC_ABI
#endif
// MinGW and Cygwin ABI
#if (((defined (_WIN32) || defined (_WIN64)) && defined (__GNUC__)) || \
     (defined (__MINGW32__) || defined (__CYGWIN__))) && \
     (defined (PARSER_JIT_X86) || defined(PARSER_JIT_X64))
#define PARSER_JIT_MINGW_ABI
#endif
// Only one ABI must be used!
#if (defined (PARSER_JIT_SYSV_ABI) && defined (PARSER_JIT_MSVC_ABI)) || \
    (defined (PARSER_JIT_SYSV_ABI) && defined (PARSER_JIT_MINGW_ABI)) || \
    (defined (PARSER_JIT_MSVC_ABI) && defined (PARSER_JIT_MINGW_ABI))
#if defined (PARSER_JIT_SYSV_ABI)
#undef PARSER_JIT_SYSV_ABI
#endif // defined PARSER_JIT_SYSV_ABI
#if defined (PARSER_JIT_MSVC_ABI)
#undef PARSER_JIT_MSVC_ABI
#endif // defined PARSER_JIT_MSVC_ABI
#if defined (PARSER_JIT_MINGW_ABI)
#undef PARSER_JIT_MINGW_ABI
#endif // defined PARSER_JIT_MINGW_ABI
#endif
#endif // !defined PARSER_JIT_DISABLE

#if !defined PARSER_JIT_DISABLE

// Internal parser's functions for opcode generation
namespace parser_opcodes
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

    // initialize 87
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

    // push, 0 := mem
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
        debug_asm_output("fld\t%cword ptr ds:[%08xh]\n",
                         (typeid(T) == typeid(float) ? 'd' : 'q'), (size_t)ptr);
#elif defined PARSER_JIT_X64
        // mov    rdx, 0aaaaaaaaaaaaaaah
        *(code_curr++) = '\x48';
        *(code_curr++) = '\xba';
        const char * tmp_mem = reinterpret_cast<const char *>(ptr);
        memcpy(code_curr, & tmp_mem, sizeof(T*));
        code_curr += sizeof(T*);
        debug_asm_output("mov\trdx, %016xh\n", (size_t)ptr);
        // fld    [dq]word ptr [rdx]
        if(typeid(T) == typeid(float))
            *(code_curr++) = '\xd9';
        else
            *(code_curr++) = '\xdd';
        *(code_curr++) = '\x02';
        debug_asm_output("fld\t%cword ptr [rdx]\n", (typeid(T) == typeid(float) ? 'd' : 'q'));
#endif
    }

    // mem := 0, pop
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
        debug_asm_output("fstp\t%cword ptr ds:[%08xh]\n", (typeid(T) == typeid(float) ? 'd' : 'q'), (size_t)ptr);
#elif defined PARSER_JIT_X64
        // mov    rdx, 0aaaaaaaaaaaaaaah
        *(code_curr++) = '\x48';
        *(code_curr++) = '\xba';
        const char * tmp_mem = reinterpret_cast<const char *>(ptr);
        memcpy(code_curr, & tmp_mem, sizeof(T*));
        code_curr += sizeof(T*);
        debug_asm_output("mov\trdx, %016xh\n", (size_t)ptr);
        // fstp    [dq]word ptr [rdx]
        if(typeid(T) == typeid(float))
            *(code_curr++) = '\xd9';
        else
            *(code_curr++) = '\xdd';
        *(code_curr++) = '\x1a';
        debug_asm_output("fstp\t%cword ptr [rdx]\n", (typeid(T) == typeid(float) ? 'd' : 'q'));
#endif
    }

    // 1 := 1 + 0, pop
    inline void fadd(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xc1';
        debug_asm_output("fadd\n");
    }

    // 1 := 1 - 0, pop
    inline void fsub(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xe9';
        debug_asm_output("fsub\n");
    }

    // 1 := 0 - 1, pop
    inline void fsubr(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xe1';
        debug_asm_output("fsubr\n");
    }

    // 1 := 1 * 0, pop
    inline void fmul(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xc9';
        debug_asm_output("fmul\n");
    }

    // 1 := 1 / 0, pop
    inline void fdiv(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xf9';
        debug_asm_output("fdiv\n");
    }

    // 1 := 0 / 1, pop
    inline void fdivr(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xf1';
        debug_asm_output("fdivr\n");
    }

    // 0 := 1 * log base 2.0 of 0, pop
    inline void fyl2x(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf1';
        debug_asm_output("fyl2x\n");
    }

    // 0 := (2.0 ** 0) - 1.0
    inline void f2xm1(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf0';
        debug_asm_output("f2xm1\n");
    }

    // push, 0 := 1.0
    inline void fld1(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xe8';
        debug_asm_output("fld1\n");
    }

    // push, 0 := Pi
    inline void fldpi(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xeb';
        debug_asm_output("fldpi\n");
    }

    // push, 0 := +0.0
    inline void fldz(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xee';
        debug_asm_output("fldz\n");
    }

    // exchange 0 and 1
    inline void fxch(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xc9';
        debug_asm_output("fxch\n");
    }

    // exchange 0 and i
    inline void fxch(char *& code_curr, int i)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xc8' + i;
        debug_asm_output("fxch\t%d\n", i);
    }

    // 0 := 0 * 2.0 ** 1
    inline void fscale(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfd';
        debug_asm_output("fscale\n");
    }

    // 387 only: push, 1/0 := sine(old 0)
    inline void fsin(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfe';
        debug_asm_output("fsin\n");
    }

    // 387 only: push, 1/0 := cosine(old 0)
    inline void fcos(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xff';
        debug_asm_output("fcos\n");
    }

    // 387 only: push, 1 := sine, 0 := cos(old 0)
    inline void fsincos(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfb';
        debug_asm_output("fsincos\n");
    }

    // 0 := square root of 0
    inline void fsqrt(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfa';
        debug_asm_output("fsqrt\n");
    }

    // push, 1/0 := tan(old 0)
    inline void fptan(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf2';
        debug_asm_output("fptan\n");
    }

    // 0 := arctan(1/0), pop
    inline void fpatan(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf3';
        debug_asm_output("fpatan\n");
    }

    // push, 0 := log base 2.0 of e
    inline void fldl2e(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xea';
        debug_asm_output("fldl2e\n");
    }

    // push, 0 := log base 2.0 of 10.0
    inline void fldl2t(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xe9';
        debug_asm_output("fldl2t\n");
    }

    // 0 := |0|
    inline void fabs(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xe1';
        debug_asm_output("fabs\n");
    }

    // push, 0 := log base e of 2.0
    inline void fldln2(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xed';
        debug_asm_output("fldln2\n");
    }

    // 0 := round(0)
    inline void frndint(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfc';
        debug_asm_output("frndint\n");
    }

    // push, 0 := old i
    inline void fldi(char *& code_curr, int i)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xc0' + i;
        debug_asm_output("fldi\t%d\n", i);
    }

    // i := 0, pop
    inline void fstpi(char *& code_curr, int i)
    {
        *(code_curr++) = '\xdd';
        *(code_curr++) = '\xd8' + i;
        debug_asm_output("fstpi\t%d\n", i);
    }

    // 0 := -0
    inline void fchs(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xe0';
        debug_asm_output("fchs\n");
    }

    // push, 1 := expo, 0 := sig
    inline void fxtract(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf4';
        debug_asm_output("fxtract\n");
    }

    // compare 0 - 0.0
    inline void ftst(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xe4';
        debug_asm_output("ftst\n");
    }

    // AX := status word
    inline void fstsw_ax(char *& code_curr)
    {
        *(code_curr++) = '\xdf';
        *(code_curr++) = '\xe0';
        debug_asm_output("fstsw\tax\n");
    }

    // Store AH into flags SF ZF xx AF xx PF xx CF
    inline void sahf(char *& code_curr)
    {
        *(code_curr++) = '\x9e';
        debug_asm_output("sahf\n");
    }

    // compare 0 - 1, pop
    inline void fcomp(char *& code_curr)
    {
        *(code_curr++) = '\xd8';
        *(code_curr++) = '\xd9';
        debug_asm_output("fcomp\n");
    }

    // Set CF to 1
    inline void stc(char *& code_curr)
    {
        *(code_curr++) = '\xf9';
        debug_asm_output("stc\n");
    }

    // 1-byte jump if ==
    inline void jz(char *& code_curr, char * code_jump, const char * label_text)
    {
        if(code_jump)
        {
            size_t diff = (size_t)code_jump - (size_t)code_curr - 2;
            if(diff > 127)
                debug_asm_output("; Error: jump size is %lu\n", (unsigned long)diff);
            *(code_curr++) = '\x74';
            memcpy(code_curr, & diff, 1);
            code_curr++;
            debug_asm_output("%s:\n", label_text);
        }
        else
        {
            code_curr += 2;
            debug_asm_output("jz\t%s\n", label_text);
        }
    }

    // 4-byte jump if ==
    inline void jz_long(char *& code_curr, char * code_jump, const char * label_text)
    {
        if(code_jump)
        {
            size_t diff = (size_t)code_jump - (size_t)code_curr - 6;
            *(code_curr++) = '\x0f';
            *(code_curr++) = '\x84';
            memcpy(code_curr, & diff, 4);
            code_curr += 4;
            debug_asm_output("%s:\n", label_text);
        }
        else
        {
            code_curr += 6;
            debug_asm_output("jz\t%s\n", label_text);
        }
    }

    // 1-byte jump if <
    inline void ja(char *& code_curr, char * code_jump, const char * label_text)
    {
        if(code_jump)
        {
            size_t diff = (size_t)code_jump - (size_t)code_curr - 2;
            if(diff > 127)
                debug_asm_output("; Error: jump size is %lu\n", (unsigned long)diff);
            *(code_curr++) = '\x77';
            memcpy(code_curr, & diff, 1);
            code_curr++;
            debug_asm_output("%s:\n", label_text);
        }
        else
        {
            code_curr += 2;
            debug_asm_output("ja\t%s\n", label_text);
        }
    }

    // 4-byte jump if <
    inline void ja_long(char *& code_curr, char * code_jump, const char * label_text)
    {
        if(code_jump)
        {
            size_t diff = (size_t)code_jump - (size_t)code_curr - 6;
            *(code_curr++) = '\x0f';
            *(code_curr++) = '\x87';
            memcpy(code_curr, & diff, 4);
            code_curr += 4;
            debug_asm_output("%s:\n", label_text);
        }
        else
        {
            code_curr += 6;
            debug_asm_output("ja\t%s\n", label_text);
        }
    }

    // 1-byte jump if >
    inline void jb(char *& code_curr, char * code_jump, const char * label_text)
    {
        if(code_jump)
        {
            size_t diff = (size_t)code_jump - (size_t)code_curr - 2;
            if(diff > 127)
                debug_asm_output("; Error: jump size is %lu\n", (unsigned long)diff);
            *(code_curr++) = '\x72';
            memcpy(code_curr, & diff, 1);
            code_curr++;
            debug_asm_output("%s:\n", label_text);
        }
        else
        {
            code_curr += 2;
            debug_asm_output("jb\t%s\n", label_text);
        }
    }

    // 4-byte jump if >
    inline void jb_long(char *& code_curr, char * code_jump, const char * label_text)
    {
        if(code_jump)
        {
            size_t diff = (size_t)code_jump - (size_t)code_curr - 6;
            *(code_curr++) = '\x0f';
            *(code_curr++) = '\x82';
            memcpy(code_curr, & diff, 4);
            code_curr += 4;
            debug_asm_output("%s:\n", label_text);
        }
        else
        {
            code_curr += 6;
            debug_asm_output("jb\t%s\n", label_text);
        }
    }

    // 1-byte jump if !=
    inline void jnz(char *& code_curr, char * code_jump, const char * label_text)
    {
        if(code_jump)
        {
            size_t diff = (size_t)code_jump - (size_t)code_curr - 2;
            if(diff > 127)
                debug_asm_output("; Error: jump size is %lu\n", (unsigned long)diff);
            *(code_curr++) = '\x75';
            memcpy(code_curr, & diff, 1);
            code_curr++;
            debug_asm_output("%s:\n", label_text);
        }
        else
        {
            code_curr += 2;
            debug_asm_output("jnz\t%s\n", label_text);
        }
    }

    // 4-byte jump if !=
    inline void jnz_long(char *& code_curr, char * code_jump, const char * label_text)
    {
        if(code_jump)
        {
            size_t diff = (size_t)code_jump - (size_t)code_curr - 6;
            *(code_curr++) = '\x0f';
            *(code_curr++) = '\x85';
            memcpy(code_curr, & diff, 4);
            code_curr += 4;
            debug_asm_output("%s:\n", label_text);
        }
        else
        {
            code_curr += 6;
            debug_asm_output("jnz\t%s\n", label_text);
        }
    }

    // 1-byte jump
    inline void jmp(char *& code_curr, char * code_jump, const char * label_text)
    {
        if(code_jump)
        {
            size_t diff = (size_t)code_jump - (size_t)code_curr - 2;
            if(diff > 127)
                debug_asm_output("; Error: jump size is %lu\n", (unsigned long)diff);
            *(code_curr++) = '\xeb';
            memcpy(code_curr, & diff, 1);
            code_curr++;
            debug_asm_output("%s:\n", label_text);
        }
        else
        {
            code_curr += 2;
            debug_asm_output("jmp\t%s\n", label_text);
        }
    }

    // 4-byte jump
    inline void jmp_long(char *& code_curr, char * code_jump, const char * label_text)
    {
        if(code_jump)
        {
            size_t diff = (size_t)code_jump - (size_t)code_curr - 5;
            *(code_curr++) = '\xe9';
            memcpy(code_curr, & diff, 4);
            code_curr += 4;
            debug_asm_output("%s:\n", label_text);
        }
        else
        {
            code_curr += 5;
            debug_asm_output("jmp\t%s\n", label_text);
        }
    }

    // mov	bl,ah
    inline void mov_bl_ah(char *& code_curr)
    {
        *(code_curr++) = '\x88';
        *(code_curr++) = '\xe3';
        debug_asm_output("mov\tbl,ah\n");
    }

    // test	bl,1
    inline void test_bl_1(char *& code_curr)
    {
        *(code_curr++) = '\xf6';
        *(code_curr++) = '\xc3';
        *(code_curr++) = '\x01';
        debug_asm_output("test\tbl,1\n");
    }

    // fprem
    inline void fprem(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf8';
        debug_asm_output("fprem\n");
    }

    // push eax(rax)
    inline void push_eax(char *& code_curr)
    {
        *(code_curr++) = '\x50';
#if defined PARSER_JIT_X86
        debug_asm_output("push\teax\n");
#elif defined PARSER_JIT_X64
        debug_asm_output("push\trax\n");
#endif
    }

    // push ebx(rbx)
    inline void push_ebx(char *& code_curr)
    {
        *(code_curr++) = '\x53';
#if defined PARSER_JIT_X86
        debug_asm_output("push\tebx\n");
#elif defined PARSER_JIT_X64
        debug_asm_output("push\trbx\n");
#endif
    }

    // pop eax(rax)
    inline void pop_eax(char *& code_curr)
    {
        *(code_curr++) = '\x58';
#if defined PARSER_JIT_X86
        debug_asm_output("pop\teax\n");
#elif defined PARSER_JIT_X64
        debug_asm_output("pop\trax\n");
#endif
    }

    // pop ebx(rbx)
    inline void pop_ebx(char *& code_curr)
    {
        *(code_curr++) = '\x5b';
#if defined PARSER_JIT_X86
        debug_asm_output("pop\tebx\n");
#elif defined PARSER_JIT_X64
        debug_asm_output("pop\trbx\n");
#endif
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

