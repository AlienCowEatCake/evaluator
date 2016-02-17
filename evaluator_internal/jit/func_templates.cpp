#include "func_templates.h"

namespace evaluator_internal_jit
{

void EVALUATOR_JIT_CALL test_func_flt()
{
    typedef float T;
    typedef T(* F)(const T &);
#if defined(EVALUATOR_JIT_X86) || defined(EVALUATOR_JIT_X32)
    (*((T *)(0xC0FFEE03))) = ((F)(0xDEADBEEF))((*((T *)(0xC0FFEE01))));
#elif defined(EVALUATOR_JIT_X64)
    (*((T *)(0xDEADBABEC0FFEE03))) = ((F)(0xDEADBABEDEADBEEF))((*((T *)(0xDEADBABEC0FFEE01))));
#endif
}

void EVALUATOR_JIT_CALL test_func_dbl()
{
    typedef double T;
    typedef T(* F)(const T &);
#if defined(EVALUATOR_JIT_X86) || defined(EVALUATOR_JIT_X32)
    (*((T *)(0xC0FFEE03))) = ((F)(0xDEADBEEF))((*((T *)(0xC0FFEE01))));
#elif defined(EVALUATOR_JIT_X64)
    (*((T *)(0xDEADBABEC0FFEE03))) = ((F)(0xDEADBABEDEADBEEF))((*((T *)(0xDEADBABEC0FFEE01))));
#endif
}

void EVALUATOR_JIT_CALL test_func_cflt()
{
    typedef std::complex<float> T;
    typedef T(* F)(const T &);
#if defined(EVALUATOR_JIT_X86) || defined(EVALUATOR_JIT_X32)
    (*((T *)(0xC0FFEE03))) = ((F)(0xDEADBEEF))((*((T *)(0xC0FFEE01))));
#elif defined(EVALUATOR_JIT_X64)
    (*((T *)(0xDEADBABEC0FFEE03))) = ((F)(0xDEADBABEDEADBEEF))((*((T *)(0xDEADBABEC0FFEE01))));
#endif
}

void EVALUATOR_JIT_CALL test_func_cdbl()
{
    typedef std::complex<double> T;
    typedef T(* F)(const T &);
#if defined(EVALUATOR_JIT_X86) || defined(EVALUATOR_JIT_X32)
    (*((T *)(0xC0FFEE03))) = ((F)(0xDEADBEEF))((*((T *)(0xC0FFEE01))));
#elif defined(EVALUATOR_JIT_X64)
    (*((T *)(0xDEADBABEC0FFEE03))) = ((F)(0xDEADBABEDEADBEEF))((*((T *)(0xDEADBABEC0FFEE01))));
#endif
}

// =============================================================================================

#if defined(EVALUATOR_JIT_X86)
    #if defined(EVALUATOR_JIT_SYSV_ABI)

        const char * code_func_flt =
                "\x83\xec\x18"             // sub    $0x18,%esp
                "\xb8\xef\xbe\xad\xde"     // mov    $0xdeadbeef,%eax
                "\x68\x01\xee\xff\xc0"     // push   $0xc0ffee01
                "\xff\xd0"                 // call   *%eax
                "\xd9\x1d\x03\xee\xff\xc0" // fstps  0xc0ffee03
                "\x83\xc4\x1c"             // add    $0x1c,%esp
                "\xc3";                    // ret

        const char * code_func_dbl =
                "\x83\xec\x18"             // sub    $0x18,%esp
                "\xb8\xef\xbe\xad\xde"     // mov    $0xdeadbeef,%eax
                "\x68\x01\xee\xff\xc0"     // push   $0xc0ffee01
                "\xff\xd0"                 // call   *%eax
                "\xdd\x1d\x03\xee\xff\xc0" // fstpl  0xc0ffee03
                "\x83\xc4\x1c"             // add    $0x1c,%esp
                "\xc3";                    // ret

        const char * code_func_cflt =
                "\x83\xec\x1c"             // sub    $0x1c,%esp
                "\x8d\x44\x24\x08"         // lea    0x8(%esp),%eax
                "\x83\xec\x08"             // sub    $0x8,%esp
                "\x68\x01\xee\xff\xc0"     // push   $0xc0ffee01
                "\x50"                     // push   %eax
                "\xb8\xef\xbe\xad\xde"     // mov    $0xdeadbeef,%eax
                "\xff\xd0"                 // call   *%eax
                "\xd9\x44\x24\x14"         // flds   0x14(%esp)
                "\xd9\x1d\x03\xee\xff\xc0" // fstps  0xc0ffee03
                "\xd9\x44\x24\x18"         // flds   0x18(%esp)
                "\xd9\x1d\x07\xee\xff\xc0" // fstps  0xc0ffee07
                "\x83\xc4\x28"             // add    $0x28,%esp
                "\xc3";                    // ret

        const char * code_func_cdbl =
                "\x83\xec\x1c"             // sub    $0x1c,%esp
                "\x89\xe0"                 // mov    %esp,%eax
                "\x83\xec\x08"             // sub    $0x8,%esp
                "\x68\x01\xee\xff\xc0"     // push   $0xc0ffee01
                "\x50"                     // push   %eax
                "\xb8\xef\xbe\xad\xde"     // mov    $0xdeadbeef,%eax
                "\xff\xd0"                 // call   *%eax
                "\xdd\x44\x24\x0c"         // fldl   0xc(%esp)
                "\xdd\x1d\x03\xee\xff\xc0" // fstpl  0xc0ffee03
                "\xdd\x44\x24\x14"         // fldl   0x14(%esp)
                "\xdd\x1d\x0b\xee\xff\xc0" // fstpl  0xc0ffee0b
                "\x83\xc4\x28"             // add    $0x28,%esp
                "\xc3";                    // ret

    #elif defined(EVALUATOR_JIT_MSVC_ABI) || defined(EVALUATOR_JIT_MINGW_ABI)

        const char * code_func_flt =
                "\x68\x01\xEE\xFF\xC0"     // push        0C0FFEE01h
                "\xB8\xEF\xBE\xAD\xDE"     // mov         eax,0DEADBEEFh
                "\xFF\xD0"                 // call        eax
                "\xD9\x1D\x03\xEE\xFF\xC0" // fstp        dword ptr ds:[0C0FFEE03h]
                "\x83\xC4\x04"             // add         esp,4
                "\xC3";                    // ret

        const char * code_func_dbl =
                "\x68\x01\xEE\xFF\xC0"     // push        0C0FFEE01h
                "\xB8\xEF\xBE\xAD\xDE"     // mov         eax,0DEADBEEFh
                "\xFF\xD0"                 // call        eax
                "\xDD\x1D\x03\xEE\xFF\xC0" // fstp        qword ptr ds:[0C0FFEE03h]
                "\x83\xC4\x04"             // add         esp,4
                "\xC3";                    // ret

        #if defined(EVALUATOR_JIT_MSVC_ABI)

        const char * code_func_cflt =
                "\x55"                     // push        ebp
                "\x8B\xEC"                 // mov         ebp,esp
                "\x83\xEC\x08"             // sub         esp,8
                "\x8D\x45\xF8"             // lea         eax,[ebp-8]
                "\x68\x01\xEE\xFF\xC0"     // push        0C0FFEE01h
                "\x50"                     // push        eax
                "\xB8\xEF\xBE\xAD\xDE"     // mov         eax,0DEADBEEFh
                "\xFF\xD0"                 // call        eax
                "\x83\xC4\x08"             // add         esp,8
                "\x8B\x08"                 // mov         ecx,dword ptr [eax]
                "\x89\x0D\x03\xEE\xFF\xC0" // mov         dword ptr ds:[0C0FFEE03h],ecx
                "\x8B\x40\x04"             // mov         eax,dword ptr [eax+4]
                "\xA3\x07\xEE\xFF\xC0"     // mov         dword ptr ds:[C0FFEE07h],eax
                "\x8B\xE5"                 // mov         esp,ebp
                "\x5D"                     // pop         ebp
                "\xC3";                    // ret

        #elif defined(EVALUATOR_JIT_MINGW_ABI)

        const char * code_func_cflt =
                "\x53"                          // push   %ebx
                "\xb8\xef\xbe\xad\xde"          // mov    $0xdeadbeef,%eax
                "\x83\xec\x18"                  // sub    $0x18,%esp
                "\xc7\x04\x24\x01\xee\xff\xc0"  // movl   $0xc0ffee01,(%esp)
                "\xff\xd0"                      // call   *%eax
                "\x89\x44\x24\x0c"              // mov    %eax,0xc(%esp)
                "\xd9\x44\x24\x0c"              // flds   0xc(%esp)
                "\x89\x54\x24\x0c"              // mov    %edx,0xc(%esp)
                "\xd9\x44\x24\x0c"              // flds   0xc(%esp)
                "\xd9\xc9"                      // fxch   %st(1)
                "\xd9\x1d\x03\xee\xff\xc0"      // fstps  0xc0ffee03
                "\xd9\x1d\x07\xee\xff\xc0"      // fstps  0xc0ffee07
                "\x83\xc4\x18"                  // add    $0x18,%esp
                "\x5b"                          // pop    %ebx
                "\xc3";                         // ret

        #else

        const char * code_func_cflt = NULL;

        #endif

        const char * code_func_cdbl =
                "\x55"                 // push        ebp
                "\x8B\xEC"             // mov         ebp,esp
                "\x83\xEC\x10"         // sub         esp,10h
                "\x8D\x45\xF0"         // lea         eax,[ebp-10h]
                "\x68\x01\xEE\xFF\xC0" // push        0C0FFEE01h
                "\x50"                 // push        eax
                "\xB8\xEF\xBE\xAD\xDE" // mov         eax,0DEADBEEFh
                "\xFF\xD0"             // call        eax
                "\xBA\x03\xEE\xFF\xC0" // mov         edx,0C0FFEE03h
                "\x83\xC4\x08"         // add         esp,8
                "\x8B\x08"             // mov         ecx,dword ptr [eax]
                "\x89\x0A"             // mov         dword ptr [edx],ecx
                "\x8B\x48\x04"         // mov         ecx,dword ptr [eax+4]
                "\x89\x4A\x04"         // mov         dword ptr [edx+4],ecx
                "\x8B\x48\x08"         // mov         ecx,dword ptr [eax+8]
                "\x89\x4A\x08"         // mov         dword ptr [edx+8],ecx
                "\x8B\x40\x0C"         // mov         eax,dword ptr [eax+0Ch]
                "\x89\x42\x0C"         // mov         dword ptr [edx+0Ch],eax
                "\x8B\xE5"             // mov         esp,ebp
                "\x5D"                 // pop         ebp
                "\xC3";                // ret

    #else

        const char * code_func_flt = NULL;
        const char * code_func_dbl = NULL;
        const char * code_func_cflt = NULL;
        const char * code_func_cdbl = NULL;

    #endif

#elif defined(EVALUATOR_JIT_X64)
    #if defined(EVALUATOR_JIT_SYSV_ABI)

        const char * code_func_flt =
                "\x48\x83\xec\x08"                         // sub    $0x8,%rsp
                "\x48\xb8\xef\xbe\xad\xde\xbe\xba\xad\xde" // movabs $0xdeadbabedeadbeef,%rax
                "\x48\xbf\x01\xee\xff\xc0\xbe\xba\xad\xde" // movabs $0xdeadbabec0ffee01,%rdi
                "\xff\xd0"                                 // callq  *%rax
                "\x48\xb8\x03\xee\xff\xc0\xbe\xba\xad\xde" // movabs $0xdeadbabec0ffee03,%rax
                "\xf3\x0f\x11\x00"                         // movss  %xmm0,(%rax)
                "\x48\x83\xc4\x08"                         // add    $0x8,%rsp
                "\xc3";                                    // retq

        const char * code_func_dbl =
                "\x48\x83\xec\x08"                         // sub    $0x8,%rsp
                "\x48\xb8\xef\xbe\xad\xde\xbe\xba\xad\xde" // movabs $0xdeadbabedeadbeef,%rax
                "\x48\xbf\x01\xee\xff\xc0\xbe\xba\xad\xde" // movabs $0xdeadbabec0ffee01,%rdi
                "\xff\xd0"                                 // callq  *%rax
                "\x48\xb8\x03\xee\xff\xc0\xbe\xba\xad\xde" // movabs $0xdeadbabec0ffee03,%rax
                "\xf2\x0f\x11\x00"                         // movsd  %xmm0,(%rax)
                "\x48\x83\xc4\x08"                         // add    $0x8,%rsp
                "\xc3";                                    // retq

        const char * code_func_cflt =
                "\x48\x83\xec\x08"                         // sub    $0x8,%rsp
                "\x48\xb8\xef\xbe\xad\xde\xbe\xba\xad\xde" // movabs $0xdeadbabedeadbeef,%rax
                "\x48\xbf\x01\xee\xff\xc0\xbe\xba\xad\xde" // movabs $0xdeadbabec0ffee01,%rdi
                "\xff\xd0"                                 // callq  *%rax
                "\x66\x48\x0f\x7e\xc0"                     // movq   %xmm0,%rax
                "\x48\xa3\x03\xee\xff\xc0\xbe\xba\xad\xde" // movabs %rax,0xdeadbabec0ffee03
                "\x48\x83\xc4\x08"                         // add    $0x8,%rsp
                "\xc3";                                    // retq

        const char * code_func_cdbl =
                "\x48\x83\xec\x08"                         // sub    $0x8,%rsp
                "\x48\xb8\xef\xbe\xad\xde\xbe\xba\xad\xde" // movabs $0xdeadbabedeadbeef,%rax
                "\x48\xbf\x01\xee\xff\xc0\xbe\xba\xad\xde" // movabs $0xdeadbabec0ffee01,%rdi
                "\xff\xd0"                                 // callq  *%rax
                "\x48\xb8\x03\xee\xff\xc0\xbe\xba\xad\xde" // movabs $0xdeadbabec0ffee03,%rax
                "\xf2\x0f\x11\x00"                         // movsd  %xmm0,(%rax)
                "\xf2\x0f\x11\x48\x08"                     // movsd  %xmm1,0x8(%rax)
                "\x48\x83\xc4\x08"                         // add    $0x8,%rsp
                "\xc3";                                    // retq

    #elif defined(EVALUATOR_JIT_MSVC_ABI) || defined(EVALUATOR_JIT_MINGW_ABI)

        const char * code_func_flt =
                "\x48\x83\xEC\x28"                         // sub         rsp,28h
                "\x48\xB9\x01\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rcx,0DEADBABEC0FFEE01h
                "\x48\xB8\xEF\xBE\xAD\xDE\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEDEADBEEFh
                "\xFF\xD0"                                 // call        rax
                "\x48\xB8\x03\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEC0FFEE03h
                "\xF3\x0F\x11\x00"                         // movss       dword ptr [rax],xmm0
                "\x48\x83\xC4\x28"                         // add         rsp,28h
                "\xC3";                                    // ret

        const char * code_func_dbl =
                "\x48\x83\xEC\x28"                         // sub         rsp,28h
                "\x48\xB9\x01\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rcx,0DEADBABEC0FFEE01h
                "\x48\xB8\xEF\xBE\xAD\xDE\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEDEADBEEFh
                "\xFF\xD0"                                 // call        rax
                "\x48\xB8\x03\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEC0FFEE03h
                "\xF2\x0F\x11\x00"                         // movsd       mmword ptr [rax],xmm0
                "\x48\x83\xC4\x28"                         // add         rsp,28h
                "\xC3";                                    // ret

        #if defined(EVALUATOR_JIT_MSVC_ABI)

        const char * code_func_cflt =
                "\x48\x83\xEC\x28"                         // sub         rsp,28h
                "\x48\xBA\x01\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rdx,0DEADBABEC0FFEE01h
                "\x48\x8D\x4C\x24\x30"                     // lea         rcx,[rsp+30h]
                "\x48\xB8\xEF\xBE\xAD\xDE\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEDEADBEEFh
                "\xFF\xD0"                                 // call        rax
                "\x48\xB9\x03\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rcx,0DEADBABEC0FFEE03h
                "\xF2\x0F\x10\x00"                         // movsd       xmm0,mmword ptr [rax]
                "\xF2\x0F\x11\x01"                         // movsd       mmword ptr [rcx],xmm0
                "\x48\x83\xC4\x28"                         // add         rsp,28h
                "\xC3";                                    // ret

        #elif defined(EVALUATOR_JIT_MINGW_ABI)

        const char * code_func_cflt =
                "\x48\x83\xec\x38"                         // sub    $0x38,%rsp
                "\x48\xb9\x01\xee\xff\xc0\xbe\xba\xad\xde" // movabs $0xdeadbabec0ffee01,%rcx
                "\x48\xb8\xef\xbe\xad\xde\xbe\xba\xad\xde" // movabs $0xdeadbabedeadbeef,%rax
                "\xff\xd0"                                 // callq  *%rax
                "\x89\x44\x24\x2c"                         // mov    %eax,0x2c(%rsp)
                "\x48\xc1\xe8\x20"                         // shr    $0x20,%rax
                "\xf3\x0f\x10\x4c\x24\x2c"                 // movss  0x2c(%rsp),%xmm1
                "\x89\x44\x24\x2c"                         // mov    %eax,0x2c(%rsp)
                "\x48\xb8\x03\xee\xff\xc0\xbe\xba\xad\xde" // movabs $0xdeadbabec0ffee03,%rax
                "\xf3\x0f\x10\x44\x24\x2c"                 // movss  0x2c(%rsp),%xmm0
                "\xf3\x0f\x11\x40\x04"                     // movss  %xmm0,0x4(%rax)
                "\xf3\x0f\x11\x08"                         // movss  %xmm1,(%rax)
                "\x48\x83\xc4\x38"                         // add    $0x38,%rsp
                "\xc3";                                    // retq

        #else

        const char * code_func_cflt = NULL;

        #endif


        const char * code_func_cdbl =
                "\x48\x83\xEC\x38"                         // sub         rsp,38h
                "\x48\xBA\x01\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rdx,0DEADBABEC0FFEE01h
                "\x48\x8D\x4C\x24\x20"                     // lea         rcx,[rsp+20h]
                "\x48\xB8\xEF\xBE\xAD\xDE\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEDEADBEEFh
                "\xFF\xD0"                                 // call        rax
                "\x48\xB9\x03\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rcx,0DEADBABEC0FFEE03h
                "\x0F\x10\x00"                             // movups      xmm0,xmmword ptr [rax]
                "\x0F\x11\x01"                             // movups      xmmword ptr [rcx],xmm0
                "\x48\x83\xC4\x38"                         // add         rsp,38h
                "\xC3";                                    // ret

    #else

        const char * code_func_flt = NULL;
        const char * code_func_dbl = NULL;
        const char * code_func_cflt = NULL;
        const char * code_func_cdbl = NULL;

    #endif

#elif defined(EVALUATOR_JIT_X32)
    #if defined(EVALUATOR_JIT_SYSV_ABI)

        const char * code_func_flt =
                "\x83\xec\x08"             // sub    $0x8,%esp
                "\xb8\xef\xbe\xad\xde"     // mov    $0xdeadbeef,%eax
                "\xbf\x01\xee\xff\xc0"     // mov    $0xc0ffee01,%edi
                "\xff\xd0"                 // callq  *%rax
                "\xb8\x03\xee\xff\xc0"     // mov    $0xc0ffee03,%eax
                "\x67\xf3\x0f\x11\x00"     // movss  %xmm0,(%eax)
                "\x83\xc4\x08"             // add    $0x8,%esp
                "\xc3";                    // retq

        const char * code_func_dbl =
                "\x83\xec\x08"             // sub    $0x8,%esp
                "\xb8\xef\xbe\xad\xde"     // mov    $0xdeadbeef,%eax
                "\xbf\x01\xee\xff\xc0"     // mov    $0xc0ffee01,%edi
                "\xff\xd0"                 // callq  *%rax
                "\xb8\x03\xee\xff\xc0"     // mov    $0xc0ffee03,%eax
                "\x67\xf2\x0f\x11\x00"     // movsd  %xmm0,(%eax)
                "\x83\xc4\x08"             // add    $0x8,%esp
                "\xc3";                    // retq

        const char * code_func_cflt =
                "\x83\xec\x08"             // sub    $0x8,%esp
                "\xb8\xef\xbe\xad\xde"     // mov    $0xdeadbeef,%eax
                "\xbf\x01\xee\xff\xc0"     // mov    $0xc0ffee01,%edi
                "\xff\xd0"                 // callq  *%rax
                "\xb8\x03\xee\xff\xc0"     // mov    $0xc0ffee03,%eax
                "\x67\x66\x0f\xd6\x00"     // movq   %xmm0,(%eax)
                "\x83\xc4\x08"             // add    $0x8,%esp
                "\xc3";                    // retq

        const char * code_func_cdbl =
                "\x83\xec\x08"             // sub    $0x8,%esp
                "\xb8\xef\xbe\xad\xde"     // mov    $0xdeadbeef,%eax
                "\xbf\x01\xee\xff\xc0"     // mov    $0xc0ffee01,%edi
                "\xff\xd0"                 // callq  *%rax
                "\xb8\x03\xee\xff\xc0"     // mov    $0xc0ffee03,%eax
                "\x67\xf2\x0f\x11\x00"     // movsd  %xmm0,(%eax)
                "\x67\xf2\x0f\x11\x48\x08" // movsd  %xmm1,0x8(%eax)
                "\x83\xc4\x08"             // add    $0x8,%esp
                "\xc3";                    // retq

    #else

        const char * code_func_flt = NULL;
        const char * code_func_dbl = NULL;
        const char * code_func_cflt = NULL;
        const char * code_func_cdbl = NULL;

    #endif
#else

    const char * code_func_flt = NULL;
    const char * code_func_dbl = NULL;
    const char * code_func_cflt = NULL;
    const char * code_func_cdbl = NULL;

#endif

}

