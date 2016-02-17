#ifndef COMMON_H
#define COMMON_H

//#define EVALUATOR_JIT_DISABLE


// Detection of system architecture

// x86 arch
#if (defined(_M_IX86 ) || defined(__X86__ ) || defined(__i386  ) || \
     defined(__IA32__) || defined(__I86__ ) || defined(__i386__) || \
     defined(__i486__) || defined(__i586__) || defined(__i686__))
    #define EVALUATOR_JIT_X86
#endif

// x64 arch
#if (defined(_M_X64  ) || defined(__x86_64) || defined(__x86_64__) || \
     defined(_M_AMD64) || defined(__amd64 ) || defined(__amd64__ ))
    #define EVALUATOR_JIT_X64
#endif

// x32 arch
#if (defined(__ILP32__) && defined(__x86_64__))
    #define EVALUATOR_JIT_X32
    #undef EVALUATOR_JIT_X64
#endif

// Unknown arch
#if !defined(EVALUATOR_JIT_X86) && !defined(EVALUATOR_JIT_X64) && !defined(EVALUATOR_JIT_X32)
    #define EVALUATOR_JIT_DISABLE
#endif



// Detection of compiler ABI

#if !defined(EVALUATOR_JIT_DISABLE)

    // System V ABI
    #if (defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))) && \
        !(defined(__CYGWIN__) || defined(__MINGW32__)) && \
        (defined(EVALUATOR_JIT_X86) || defined(EVALUATOR_JIT_X64) || defined(EVALUATOR_JIT_X32))
        #define EVALUATOR_JIT_SYSV_ABI
    #endif

    // MS Compiler ABI
    #if (defined(_WIN32) || defined(_WIN64)) && \
        !(defined(__CYGWIN__) || defined(__MINGW32__)) && \
        (defined(EVALUATOR_JIT_X86) || defined(EVALUATOR_JIT_X64))
        #define EVALUATOR_JIT_MSVC_ABI
    #endif

    // MinGW and Cygwin ABI
    #if (((defined(_WIN32) || defined(_WIN64)) && defined(__GNUC__)) || \
         (defined(__MINGW32__) || defined(__CYGWIN__))) && \
         (defined(EVALUATOR_JIT_X86) || defined(EVALUATOR_JIT_X64))
        #define EVALUATOR_JIT_MINGW_ABI
    #endif

    // Only one ABI must be used!
    #if (defined(EVALUATOR_JIT_SYSV_ABI) && defined(EVALUATOR_JIT_MSVC_ABI)) || \
        (defined(EVALUATOR_JIT_SYSV_ABI) && defined(EVALUATOR_JIT_MINGW_ABI)) || \
        (defined(EVALUATOR_JIT_MSVC_ABI) && defined(EVALUATOR_JIT_MINGW_ABI))
        #if defined (EVALUATOR_JIT_SYSV_ABI)
            #undef EVALUATOR_JIT_SYSV_ABI
        #endif
        #if defined (EVALUATOR_JIT_MSVC_ABI)
            #undef EVALUATOR_JIT_MSVC_ABI
        #endif
        #if defined (EVALUATOR_JIT_MINGW_ABI)
            #undef EVALUATOR_JIT_MINGW_ABI
        #endif
    #endif

#endif



// Calling convention for JIT-compiled function

#if !defined(EVALUATOR_JIT_DISABLE)

    #if defined(_MSC_VER)
        #define EVALUATOR_JIT_CALL __cdecl
    #elif defined(__GNUC__) && defined(EVALUATOR_JIT_X86)
        #define EVALUATOR_JIT_CALL __attribute__((__cdecl__))
    #else
        #define EVALUATOR_JIT_CALL
    #endif

#else

    #define EVALUATOR_JIT_CALL

#endif



// Executable memory allocation and deallocation

#include <cstring>

namespace evaluator_internal_jit
{

void * exec_alloc(size_t size);
void exec_dealloc(void * data, size_t size);

}


#endif // COMMON_H

