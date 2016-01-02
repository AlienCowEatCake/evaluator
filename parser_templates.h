#if !defined PARSER_TEMPLATES_H
#define PARSER_TEMPLATES_H

#include "parser_opcodes.h"

#if !defined PARSER_JIT_DISABLE

namespace parser_templates
{
    // =============================================================================================

    static void PARSER_JIT_CALL test_oper_float()
    {
        typedef float T;
        typedef T(* F)(const T &, const T &);
#if defined PARSER_JIT_X86
        (*((T *)(0xC0FFEE03))) = ((F)(0xDEADBEEF))((*((T *)(0xC0FFEE01))), (*((T *)(0xC0FFEE02))));
#elif defined PARSER_JIT_X64
        (*((T *)(0xDEADBABEC0FFEE03))) = ((F)(0xDEADBABEDEADBEEF))((*((T *)(0xDEADBABEC0FFEE01))),
                                                                   (*((T *)(0xDEADBABEC0FFEE02))));
#endif
    }

    static void PARSER_JIT_CALL test_oper_double()
    {
        typedef double T;
        typedef T(* F)(const T &, const T &);
#if defined PARSER_JIT_X86
        (*((T *)(0xC0FFEE03))) = ((F)(0xDEADBEEF))((*((T *)(0xC0FFEE01))), (*((T *)(0xC0FFEE02))));
#elif defined PARSER_JIT_X64
        (*((T *)(0xDEADBABEC0FFEE03))) = ((F)(0xDEADBABEDEADBEEF))((*((T *)(0xDEADBABEC0FFEE01))),
                                                                   (*((T *)(0xDEADBABEC0FFEE02))));
#endif
    }

    static void PARSER_JIT_CALL test_oper_complex_float()
    {
        typedef std::complex<float> T;
        typedef T(* F)(const T &, const T &);
#if defined PARSER_JIT_X86
        (*((T *)(0xC0FFEE03))) = ((F)(0xDEADBEEF))((*((T *)(0xC0FFEE01))), (*((T *)(0xC0FFEE02))));
#elif defined PARSER_JIT_X64
        (*((T *)(0xDEADBABEC0FFEE03))) = ((F)(0xDEADBABEDEADBEEF))((*((T *)(0xDEADBABEC0FFEE01))),
                                                                   (*((T *)(0xDEADBABEC0FFEE02))));
#endif
    }

    static void PARSER_JIT_CALL test_oper_complex_double()
    {
        typedef std::complex<double> T;
        typedef T(* F)(const T &, const T &);
#if defined PARSER_JIT_X86
        (*((T *)(0xC0FFEE03))) = ((F)(0xDEADBEEF))((*((T *)(0xC0FFEE01))), (*((T *)(0xC0FFEE02))));
#elif defined PARSER_JIT_X64
        (*((T *)(0xDEADBABEC0FFEE03))) = ((F)(0xDEADBABEDEADBEEF))((*((T *)(0xDEADBABEC0FFEE01))),
                                                                   (*((T *)(0xDEADBABEC0FFEE02))));
#endif
    }

    // =============================================================================================

    static void PARSER_JIT_CALL test_func_float()
    {
        typedef float T;
        typedef T(* F)(const T &);
#if defined PARSER_JIT_X86
        (*((T *)(0xC0FFEE03))) = ((F)(0xDEADBEEF))((*((T *)(0xC0FFEE01))));
#elif defined PARSER_JIT_X64
        (*((T *)(0xDEADBABEC0FFEE03))) = ((F)(0xDEADBABEDEADBEEF))((*((T *)(0xDEADBABEC0FFEE01))));
#endif
    }

    static void PARSER_JIT_CALL test_func_double()
    {
        typedef double T;
        typedef T(* F)(const T &);
#if defined PARSER_JIT_X86
        (*((T *)(0xC0FFEE03))) = ((F)(0xDEADBEEF))((*((T *)(0xC0FFEE01))));
#elif defined PARSER_JIT_X64
        (*((T *)(0xDEADBABEC0FFEE03))) = ((F)(0xDEADBABEDEADBEEF))((*((T *)(0xDEADBABEC0FFEE01))));
#endif
    }

    static void PARSER_JIT_CALL test_func_complex_float()
    {
        typedef std::complex<float> T;
        typedef T(* F)(const T &);
#if defined PARSER_JIT_X86
        (*((T *)(0xC0FFEE03))) = ((F)(0xDEADBEEF))((*((T *)(0xC0FFEE01))));
#elif defined PARSER_JIT_X64
        (*((T *)(0xDEADBABEC0FFEE03))) = ((F)(0xDEADBABEDEADBEEF))((*((T *)(0xDEADBABEC0FFEE01))));
#endif
    }

    static void PARSER_JIT_CALL test_func_complex_double()
    {
        typedef std::complex<double> T;
        typedef T(* F)(const T &);
#if defined PARSER_JIT_X86
        (*((T *)(0xC0FFEE03))) = ((F)(0xDEADBEEF))((*((T *)(0xC0FFEE01))));
#elif defined PARSER_JIT_X64
        (*((T *)(0xDEADBABEC0FFEE03))) = ((F)(0xDEADBABEDEADBEEF))((*((T *)(0xDEADBABEC0FFEE01))));
#endif
    }

    // =============================================================================================

    template<typename T>
    class functions_2arg_generator
    {
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
        functions_2arg_generator()
        {
            status = true;
            const char * tc = NULL;
#if !defined _MSC_VER
            if(typeid(T) == typeid(double))
            {
                void(PARSER_JIT_CALL * func)() = test_oper_double;
                tc = reinterpret_cast<const char *>(func);
            }
            else if(typeid(T) == typeid(float))
            {
                void(PARSER_JIT_CALL * func)() = test_oper_float;
                tc = reinterpret_cast<const char *>(func);
            }
            else if(typeid(T) == typeid(std::complex<double>))
            {
                void(PARSER_JIT_CALL * func)() = test_oper_complex_double;
                tc = reinterpret_cast<const char *>(func);
            }
            else if(typeid(T) == typeid(std::complex<float>))
            {
                void(PARSER_JIT_CALL * func)() = test_oper_complex_float;
                tc = reinterpret_cast<const char *>(func);
            }
#else
            // MS have Security Checks :(
            // https://msdn.microsoft.com/en-us/library/aa290051%28v=vs.71%29.aspx
            if(typeid(T) == typeid(double))
            {
                static const char ms_dbl [] =
#if defined PARSER_JIT_X86
                        "\x68\x02\xEE\xFF\xC0"     // push        0C0FFEE02h
                        "\x68\x01\xEE\xFF\xC0"     // push        0C0FFEE01h
                        "\xB8\xEF\xBE\xAD\xDE"     // mov         eax,0DEADBEEFh
                        "\xFF\xD0"                 // call        eax
                        "\xDD\x1D\x03\xEE\xFF\xC0" // fstp        qword ptr ds:[0C0FFEE03h]
                        "\x83\xC4\x08"             // add         esp,8
                        "\xC3";                    // ret
#elif defined PARSER_JIT_X64
                        "\x48\x83\xEC\x28"                         // sub         rsp,28h
                        "\x48\xBA\x02\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rdx,0DEADBABEC0FFEE02h
                        "\x48\xB9\x01\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rcx,0DEADBABEC0FFEE01h
                        "\x48\xB8\xEF\xBE\xAD\xDE\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEDEADBEEFh
                        "\xFF\xD0"                                 // call        rax
                        "\x48\xB8\x03\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEC0FFEE03h
                        "\xF2\x0F\x11\x00"                         // movsd       mmword ptr [rax],xmm0
                        "\x48\x83\xC4\x28"                         // add         rsp,28h
                        "\xC3";                                    // ret
#else
                        "\xC3"; // ret
#endif
                tc = reinterpret_cast<const char *>(ms_dbl);
            }
            else if(typeid(T) == typeid(float))
            {
                static const char ms_flt [] =
#if defined PARSER_JIT_X86
                        "\x68\x02\xEE\xFF\xC0"     // push        0C0FFEE02h
                        "\x68\x01\xEE\xFF\xC0"     // push        0C0FFEE01h
                        "\xB8\xEF\xBE\xAD\xDE"     // mov         eax,0DEADBEEFh
                        "\xFF\xD0"                 // call        eax
                        "\xD9\x1D\x03\xEE\xFF\xC0" // fstp        dword ptr ds:[0C0FFEE03h]
                        "\x83\xC4\x08"             // add         esp,8
                        "\xC3";                    // ret
#elif defined PARSER_JIT_X64
                        "\x48\x83\xEC\x28"                         // sub         rsp,28h
                        "\x48\xBA\x02\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rdx,0DEADBABEC0FFEE02h
                        "\x48\xB9\x01\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rcx,0DEADBABEC0FFEE01h
                        "\x48\xB8\xEF\xBE\xAD\xDE\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEDEADBEEFh
                        "\xFF\xD0"                                 // call        rax
                        "\x48\xB8\x03\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEC0FFEE03h
                        "\xF3\x0F\x11\x00"                         // movss       dword ptr [rax],xmm0
                        "\x48\x83\xC4\x28"                         // add         rsp,28h
                        "\xC3";                                    // ret
#else
                        "\xC3"; // ret
#endif
                tc = reinterpret_cast<const char *>(ms_flt);
            }
            else if(typeid(T) == typeid(std::complex<double>))
            {
                static const char ms_cdbl [] =
#if defined PARSER_JIT_X86
                        "\x55"                 // push        ebp
                        "\x8B\xEC"             // mov         ebp,esp
                        "\x83\xEC\x10"         // sub         esp,10h
                        "\x8D\x45\xF0"         // lea         eax,[ebp-10h]
                        "\x68\x02\xEE\xFF\xC0" // push        0C0FFEE02h
                        "\x68\x01\xEE\xFF\xC0" // push        0C0FFEE01h
                        "\x50"                 // push        eax
                        "\xB8\xEF\xBE\xAD\xDE" // mov         eax,0DEADBEEFh
                        "\xFF\xD0"             // call        eax
                        "\xBA\x03\xEE\xFF\xC0" // mov         edx,0C0FFEE03h
                        "\x83\xC4\x0C"         // add         esp,0Ch
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
#elif defined PARSER_JIT_X64
                        "\x48\x83\xEC\x38"                         // sub         rsp,38h
                        "\x48\xBA\x01\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rdx,0DEADBABEC0FFEE01h
                        "\x48\x8D\x4C\x24\x20"                     // lea         rcx,[rsp+20h]
                        "\x49\xB8\x02\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         r8,0DEADBABEC0FFEE02h
                        "\x48\xB8\xEF\xBE\xAD\xDE\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEDEADBEEFh
                        "\xFF\xD0"                                 // call        rax
                        "\x48\xB9\x03\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rcx,0DEADBABEC0FFEE03h
                        "\x0F\x10\x00"                             // movups      xmm0,xmmword ptr [rax]
                        "\x0F\x11\x01"                             // movups      xmmword ptr [rcx],xmm0
                        "\x48\x83\xC4\x38"                         // add         rsp,38h
                        "\xC3";                                    // ret
#else
                        "\xC3"; // ret
#endif
                tc = reinterpret_cast<const char *>(ms_cdbl);
            }
            else if(typeid(T) == typeid(std::complex<float>))
            {
                static const char ms_cflt [] =
#if defined PARSER_JIT_X86
                        "\x55"                     // push        ebp
                        "\x8B\xEC"                 // mov         ebp,esp
                        "\x83\xEC\x08"             // sub         esp,8
                        "\x8D\x45\xF8"             // lea         eax,[ebp-8]
                        "\x68\x02\xEE\xFF\xC0"     // push        0C0FFEE02h
                        "\x68\x01\xEE\xFF\xC0"     // push        0C0FFEE01h
                        "\x50"                     // push        eax
                        "\xB8\xEF\xBE\xAD\xDE"     // mov         eax,0DEADBEEFh
                        "\xFF\xD0"                 // call        eax
                        "\x83\xC4\x0C"             // add         esp,0Ch
                        "\x8B\x08"                 // mov         ecx,dword ptr [eax]
                        "\x89\x0D\x03\xEE\xFF\xC0" // mov         dword ptr ds:[0C0FFEE03h],ecx
                        "\x8B\x40\x04"             // mov         eax,dword ptr [eax+4]
                        "\xA3\x07\xEE\xFF\xC0"     // mov         dword ptr ds:[C0FFEE07h],eax
                        "\x8B\xE5"                 // mov         esp,ebp
                        "\x5D"                     // pop         ebp
                        "\xC3";                    // ret
#elif defined PARSER_JIT_X64
                        "\x48\x83\xEC\x28"                         // sub         rsp,28h
                        "\x48\xBA\x01\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rdx,0DEADBABEC0FFEE01h
                        "\x48\x8D\x4C\x24\x30"                     // lea         rcx,[rsp+30h]
                        "\x49\xB8\x02\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         r8,0DEADBABEC0FFEE02h
                        "\x48\xB8\xEF\xBE\xAD\xDE\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEDEADBEEFh
                        "\xFF\xD0"                                 // call        rax
                        "\x48\xB9\x03\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rcx,0DEADBABEC0FFEE03h
                        "\xF2\x0F\x10\x00"                         // movsd       xmm0,mmword ptr [rax]
                        "\xF2\x0F\x11\x01"                         // movsd       mmword ptr [rcx],xmm0
                        "\x48\x83\xC4\x28"                         // add         rsp,28h
                        "\xC3";                                    // ret
#else
                        "\xC3"; // ret
#endif
                tc = reinterpret_cast<const char *>(ms_cflt);
            }
#endif
            while(tc[0] == '\xe9') // jump
            {
                size_t offset;
                memcpy(&offset, tc + 1, 4);
                tc += offset + 5;
            }
            code_len = 0;
            const char * tcc = tc;
            while(* tcc != '\xc3') // ret
            {
                tcc++;
                code_len++;
            }
            raw_data = new char [code_len + 10 * sizeof(size_t)];
            memcpy(raw_data, tc, code_len);
            offset_return = offset_return_2 = offset_larg = offset_rarg = offset_func = 0;
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

#if defined PARSER_ASM_DEBUG
            printf("Operator template:\n");
            for(size_t i = 0; i < code_len; i++)
            {
                unsigned char q;
                memcpy(&q, raw_data + i, 1);
                printf("%02x ", q);
                fflush(stdout);
            }
            printf("\n");
            fflush(stdout);
#endif
        }
        ~functions_2arg_generator()
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

#if defined PARSER_ASM_DEBUG
#if defined PARSER_JIT_X86
            printf("; call %xh (%xh, %xh) -> %xh\n", (size_t)func, (size_t)larg, (size_t)rarg, (size_t)ret);
#elif defined PARSER_JIT_X64
            printf("; call %llxh (%llxh, %llxh) -> %llxh\n", (size_t)func, (size_t)larg, (size_t)rarg, (size_t)ret);
#endif
#endif
        }
        bool check() const
        {
            return status;
        }
    };

    // =============================================================================================

    template<typename T>
    class functions_1arg_generator
    {
    protected:
        size_t code_len;
        char * raw_data;
        size_t offset_return;
        size_t offset_return_2;
        size_t offset_arg;
        size_t offset_func;
        bool status;
    public:
        functions_1arg_generator()
        {
            status = true;
            const char * tc = NULL;
#if !defined _MSC_VER
            if(typeid(T) == typeid(double))
            {
                void(PARSER_JIT_CALL * func)() = test_func_double;
                tc = reinterpret_cast<const char *>(func);
            }
            else if(typeid(T) == typeid(float))
            {
                void(PARSER_JIT_CALL * func)() = test_func_float;
                tc = reinterpret_cast<const char *>(func);
            }
            else if(typeid(T) == typeid(std::complex<double>))
            {
                void(PARSER_JIT_CALL * func)() = test_func_complex_double;
                tc = reinterpret_cast<const char *>(func);
            }
            else if(typeid(T) == typeid(std::complex<float>))
            {
                void(PARSER_JIT_CALL * func)() = test_func_complex_float;
                tc = reinterpret_cast<const char *>(func);
            }
#else
            // MS have Security Checks :(
            // https://msdn.microsoft.com/en-us/library/aa290051%28v=vs.71%29.aspx
            if(typeid(T) == typeid(double))
            {
                static const char ms_dbl [] =
#if defined PARSER_JIT_X86
                        "\x68\x01\xEE\xFF\xC0"     // push        0C0FFEE01h
                        "\xB8\xEF\xBE\xAD\xDE"     // mov         eax,0DEADBEEFh
                        "\xFF\xD0"                 // call        eax
                        "\xDD\x1D\x03\xEE\xFF\xC0" // fstp        qword ptr ds:[0C0FFEE03h]
                        "\x83\xC4\x04"             // add         esp,4
                        "\xC3";                    // ret
#elif defined PARSER_JIT_X64
                        "\x48\x83\xEC\x28"                         // sub         rsp,28h
                        "\x48\xB9\x01\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rcx,0DEADBABEC0FFEE01h
                        "\x48\xB8\xEF\xBE\xAD\xDE\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEDEADBEEFh
                        "\xFF\xD0"                                 // call        rax
                        "\x48\xB8\x03\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEC0FFEE03h
                        "\xF2\x0F\x11\x00"                         // movsd       mmword ptr [rax],xmm0
                        "\x48\x83\xC4\x28"                         // add         rsp,28h
                        "\xC3";                                    // ret
#else
                        "\xC3"; // ret
#endif
                tc = reinterpret_cast<const char *>(ms_dbl);
            }
            else if(typeid(T) == typeid(float))
            {
                static const char ms_flt [] =
#if defined PARSER_JIT_X86
                        "\x68\x01\xEE\xFF\xC0"     // push        0C0FFEE01h
                        "\xB8\xEF\xBE\xAD\xDE"     // mov         eax,0DEADBEEFh
                        "\xFF\xD0"                 // call        eax
                        "\xD9\x1D\x03\xEE\xFF\xC0" // fstp        dword ptr ds:[0C0FFEE03h]
                        "\x83\xC4\x04"             // add         esp,4
                        "\xC3";                    // ret
#elif defined PARSER_JIT_X64
                        "\x48\x83\xEC\x28"                         // sub         rsp,28h
                        "\x48\xB9\x01\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rcx,0DEADBABEC0FFEE01h
                        "\x48\xB8\xEF\xBE\xAD\xDE\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEDEADBEEFh
                        "\xFF\xD0"                                 // call        rax
                        "\x48\xB8\x03\xEE\xFF\xC0\xBE\xBA\xAD\xDE" // mov         rax,0DEADBABEC0FFEE03h
                        "\xF3\x0F\x11\x00"                         // movss       dword ptr [rax],xmm0
                        "\x48\x83\xC4\x28"                         // add         rsp,28h
                        "\xC3";                                    // ret
#else
                        "\xC3"; // ret
#endif
                tc = reinterpret_cast<const char *>(ms_flt);
            }
            else if(typeid(T) == typeid(std::complex<double>))
            {
                static const char ms_cdbl [] =
#if defined PARSER_JIT_X86
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
#elif defined PARSER_JIT_X64
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
                        "\xC3"; // ret
#endif
                tc = reinterpret_cast<const char *>(ms_cdbl);
            }
            else if(typeid(T) == typeid(std::complex<float>))
            {
                static const char ms_cflt [] =
#if defined PARSER_JIT_X86
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
#elif defined PARSER_JIT_X64
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
#else
                        "\xC3"; // ret
#endif
                tc = reinterpret_cast<const char *>(ms_cflt);
            }
#endif
            while(tc[0] == '\xe9') // jump
            {
                size_t offset;
                memcpy(&offset, tc + 1, 4);
                tc += offset + 5;
            }
            code_len = 0;
            const char * tcc = tc;
            while(* tcc != '\xc3') // ret
            {
                tcc++;
                code_len++;
            }
            raw_data = new char [code_len + 10 * sizeof(size_t)];
            memcpy(raw_data, tc, code_len);
            offset_return = offset_return_2 = offset_arg = offset_func = 0;
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
//#if defined _MSC_VER
//                // debug call with relative offset
//                if(raw_data[i] == '\xe8')
//                    raw_data[i] = raw_data[i + 1] = raw_data[i + 2] =
//                                  raw_data[i + 3] = raw_data[i + 4] = '\x90'; // nop
//                // vs security
//                if(raw_data[i] == '\x48' && raw_data[i + 1] == '\x8b' && raw_data[i + 2] == '\x05' &&
//                   raw_data[i + 7] == '\x48' && raw_data[i + 8] == '\x33' && raw_data[i + 9] == '\xC5' &&
//                   raw_data[i + 10] == '\x48' && raw_data[i + 11] == '\x89' && raw_data[i + 12] == '\x85')
//                    for(size_t j = 0; j < 17; j++)
//                        raw_data[i + j] = '\x90'; // nop
//                if((raw_data[i] == '\x48' && raw_data[i + 1] == '\x8B' && raw_data[i + 2] == '\x8D') ||
//                   (raw_data[i] == '\x48' && raw_data[i + 1] == '\x89' && raw_data[i + 2] == '\x85') ||
//                   (raw_data[i] == '\x48' && raw_data[i + 1] == '\x8B' && raw_data[i + 2] == '\x05'))
//                    for(size_t j = 0; j < 7; j++)
//                        raw_data[i + j] = '\x90'; // nop
//#endif
            }
            if(offset_return == 0 || offset_arg == 0 || offset_func == 0)
                status = false;

#if defined PARSER_ASM_DEBUG
            printf("Function template:\n");
            for(size_t i = 0; i < code_len; i++)
            {
                unsigned char q;
                memcpy(&q, raw_data + i, 1);
                printf("%02x ", q);
                fflush(stdout);
            }
            printf("\n");
            fflush(stdout);
#endif
        }
        ~functions_1arg_generator()
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

#if defined PARSER_ASM_DEBUG
#if defined PARSER_JIT_X86
            printf("; call %xh (%xh) -> %xh\n", (size_t)func, (size_t)arg, (size_t)ret);
#elif defined PARSER_JIT_X64
            printf("; call %llxh (%llxh) -> %llxh\n", (size_t)func, (size_t)arg, (size_t)ret);
#endif
#endif
        }
        bool check() const
        {
            return status;
        }
    };

    // =============================================================================================
}

#endif

#endif // PARSER_TEMPLATES_H

