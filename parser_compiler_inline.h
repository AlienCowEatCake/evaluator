#if !defined PARSER_COMPILER_INLINE_H
#define PARSER_COMPILER_INLINE_H

#include "parser.h"

template<typename T>
bool parser<T>::compile_inline()
{
#if !defined PARSER_JIT_DISABLE
    using namespace std;
    using namespace parser_internal;
    using namespace parser_opcodes;

    if(!is_parsed())
    {
        error_string = "Not parsed!";
        return false;
    }

    if(!jit_code || !jit_code_size)
    {
        jit_code_size = 128 * 1024; // 128 KiB
#if defined _WIN32 || defined _WIN64
        jit_code = (char *)malloc(jit_code_size);
        DWORD tmp;
        VirtualProtect(jit_code, jit_code_size, PAGE_EXECUTE_READWRITE, &tmp);
#else
        int prot = PROT_READ | PROT_WRITE | PROT_EXEC;
        int flags = MAP_PRIVATE | MAP_ANONYMOUS;
        jit_code = (char *)mmap(NULL, jit_code_size, prot, flags, -1, 0);
#endif
    }
    memset(jit_code, 0, jit_code_size);

    if(!jit_stack || !jit_stack_size)
    {
        jit_stack_size = 128 * 1024 / sizeof(T); // 128 KiB
        jit_stack = new T [jit_stack_size];
    }

    char * curr = jit_code;
    T * jit_stack_curr = jit_stack;

#if defined PARSER_JIT_X86 || defined PARSER_JIT_X64

    if((typeid(T) == typeid(float) && sizeof(float) == 4) ||
       (typeid(T) == typeid(double) && sizeof(double) == 8))
    {
        char * last_push_pos = NULL;
        T * last_push_val = NULL;
        for(typename vector<parser_object<T> >::const_iterator
            it = expression.begin(); it != expression.end(); ++it)
        {
            if(it->is_constant() || it->is_variable())
            {
                fld_ptr(curr, it->raw_value());
                last_push_pos = curr;
                last_push_val = jit_stack_curr;
                fstp_ptr(curr, jit_stack_curr++);
            }
            else if(it->is_operator())
            {
                jit_stack_curr -= 2;
                if(last_push_val == jit_stack_curr + 1)
                {
                    curr = last_push_pos;
                    fld_ptr(curr, jit_stack_curr);
                    fxch(curr);
                }
                else
                {
                    fld_ptr(curr, jit_stack_curr++);
                    fld_ptr(curr, jit_stack_curr--);
                }

                string op = it->str();
                if     (op[0] == '+')
                    fadd(curr);
                else if(op[0] == '-')
                    fsub(curr);
                else if(op[0] == '*')
                    fmul(curr);
                else if(op[0] == '/')
                    fdiv(curr);
                else if(op[0] == '^')
                {
                    fxch(curr);
                    // https://stackoverflow.com/questions/4638473/how-to-powreal-real-in-x86
                    fyl2x(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr(curr, jit_stack_curr);
                }
                else
                {
                    error_string = "Unsupported operator " + it->str();
                    return false;
                }

                last_push_pos = curr;
                last_push_val = jit_stack_curr;
                fstp_ptr(curr, jit_stack_curr++);
            }
            else if(it->is_function())
            {
                jit_stack_curr--;
                if(last_push_val == jit_stack_curr)
                    curr = last_push_pos;
                else
                    fld_ptr(curr, jit_stack_curr);

                string fu = it->str();
                if     (fu == "sin")
                    fsin(curr);
                else if(fu == "cos")
                    fcos(curr);
                else if(fu == "sqrt")
                    fsqrt(curr);
                else if(fu == "tan")
                {
                    fptan(curr);
                    fdiv(curr);
                }
                else if(fu == "atan")
                {
                    fld1(curr);
                    fpatan(curr);
                }
                else if(fu == "asin")
                {
                    // arcsin(a) = arctg(a / sqrt(1 - a^2))
                    fldi(curr, 0);
                    fldi(curr, 0);
                    fmul(curr);
                    fld1(curr);
                    fsubr(curr);
                    fsqrt(curr);
                    fpatan(curr);
                }
                else if(fu == "acos")
                {
                    // arccos(a) = 2 * arctg(sqrt(1 - a) / sqrt(1 + a))
                    fldi(curr, 0);
                    fld1(curr);
                    fsubr(curr);
                    fsqrt(curr);
                    fxch(curr);
                    fld1(curr);
                    fadd(curr);
                    fsqrt(curr);
                    fpatan(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                }
                else if(fu == "log")
                {
                    fld1(curr);
                    fxch(curr);
                    fyl2x(curr);
                    fldl2e(curr);
                    fdiv(curr);
                }
                else if(fu == "log2")
                {
                    fld1(curr);
                    fxch(curr);
                    fyl2x(curr);
                }
                else if(fu == "log10")
                {
                    fld1(curr);
                    fxch(curr);
                    fyl2x(curr);
                    fldl2t(curr);
                    fdiv(curr);
                }
                else if(fu == "abs")
                    fabs(curr);
                else if(fu == "exp")
                {
                    // http://mathforum.org/kb/message.jspa?messageID=1640026
                    fldl2e(curr);
                    fmul(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr(curr, jit_stack_curr);
                }
                else if(fu == "sinh" || fu == "cosh")
                {
                    // sinh(x) = (exp(x) - exp(-x)) / 2
                    // cosh(x) = (exp(x) + exp(-x)) / 2
                    fldi(curr, 0);
                    // exp(x)
                    fldl2e(curr);
                    fmul(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr(curr, jit_stack_curr);
                    // exp(-x)
                    fxch(curr);
                    fchs(curr);
                    fldl2e(curr);
                    fmul(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr(curr, jit_stack_curr);
                    // (exp(x) +- exp(-x)) / 2
                    if(fu == "cosh")
                        fadd(curr);
                    else
                        fsub(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fdiv(curr);
                }
                else if(fu == "tanh")
                {
                    // tanh(x) = (exp(2*x) - 1) / (exp(2*x) + 1)
                    // 2*x
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    // exp(2*x)
                    fldl2e(curr);
                    fmul(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr(curr, jit_stack_curr);
                    // exp(2*x) - 1
                    fldi(curr, 0);
                    fld1(curr);
                    fsub(curr);
                    // exp(2*x) + 1
                    fxch(curr);
                    fld1(curr);
                    fadd(curr);
                    // (exp(2*x) - 1) / (exp(2*x) + 1)
                    fdiv(curr);
                }
                else if(fu == "asinh" || fu == "acosh")
                {
                    // asinh(x) = log(x + sqrt(x * x + 1))
                    // acosh(x) = log(x + sqrt(x * x - 1))
                    fldi(curr, 0);
                    fldi(curr, 0);
                    fmul(curr);
                    fld1(curr);
                    if(fu == "acosh")
                        fsub(curr);
                    else
                        fadd(curr);
                    fsqrt(curr);
                    fadd(curr);
                    // log(...)
                    fld1(curr);
                    fxch(curr);
                    fyl2x(curr);
                    fldl2e(curr);
                    fdiv(curr);
                }
                else if(fu == "atanh")
                {
                    // 0.5 * log((1.0 + x) / (1.0 - x))
                    fldi(curr, 0);
                    fld1(curr);
                    fadd(curr);
                    fxch(curr);
                    fld1(curr);
                    fsubr(curr);
                    fdiv(curr);
                    // log(...)
                    fld1(curr);
                    fxch(curr);
                    fyl2x(curr);
                    fldl2e(curr);
                    fdiv(curr);
                    // 0.5 * log(...)
                    fld1(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fdiv(curr);
                    fmul(curr);
                }
                else if(fu == "imag")
                {
                    fldz(curr);
                    fmul(curr);
                }
                else if(fu == "arg")
                {
                    // 0 if x > 0; pi if x < 0
                    fldi(curr, 0);
                    fldi(curr, 0);
                    fabs(curr);
                    fsub(curr);
                    fxch(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    fdiv(curr);
                    fabs(curr);
                    fldpi(curr);
                    fmul(curr);
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    // fldi(curr, 0);
                    // fabs(curr);
                    // fsubr(curr);
                    // fldz(curr);
                    // fpatan(curr);
                    // fld1(curr);
                    // fld1(curr);
                    // fadd(curr);
                    // fmul(curr);
                }
                else if(fu != "real" && fu != "conj")
                {
                    error_string = "Unsupported function " + it->str();
                    return false;
                }
                last_push_pos = curr;
                last_push_val = jit_stack_curr;
                fstp_ptr(curr, jit_stack_curr++);
            }
        }

        jit_stack_curr--;
    }
    else if((typeid(T) == typeid(complex<float>) && sizeof(float) == 4) ||
            (typeid(T) == typeid(complex<double>) && sizeof(double) == 8))
    {
        for(typename vector<parser_object<T> >::const_iterator
            it = expression.begin(); it != expression.end(); ++it)
        {
            if(it->is_constant() || it->is_variable())
            {
                fld_ptr_real(curr, it->raw_value());
                fstp_ptr_real(curr, jit_stack_curr);
                fld_ptr_imag(curr, it->raw_value());
                fstp_ptr_imag(curr, jit_stack_curr++);
            }
            else if(it->is_operator())
            {
                string op = it->str();
                jit_stack_curr -= 2;
                if(op[0] == '+' || op[0] == '-')
                {
                    fld_ptr_real(curr, jit_stack_curr);
                    fld_ptr_real(curr, jit_stack_curr + 1);
                    if(op[0] == '+')
                        fadd(curr);
                    else
                        fsub(curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                    fld_ptr_imag(curr, jit_stack_curr);
                    fld_ptr_imag(curr, jit_stack_curr + 1);
                    if(op[0] == '+')
                        fadd(curr);
                    else
                        fsub(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                }
                else if(op[0] == '*')
                {
                    // (a+bi)*(c+di) = (ac-bd)+(bc+ad)i
                    fld_ptr_real(curr, jit_stack_curr);
                    fld_ptr_real(curr, jit_stack_curr + 1);
                    fmul(curr);
                    fld_ptr_imag(curr, jit_stack_curr);
                    fld_ptr_imag(curr, jit_stack_curr + 1);
                    fmul(curr);
                    fsub(curr);
                    fld_ptr_real(curr, jit_stack_curr);
                    fld_ptr_imag(curr, jit_stack_curr + 1);
                    fmul(curr);
                    fld_ptr_imag(curr, jit_stack_curr);
                    fld_ptr_real(curr, jit_stack_curr + 1);
                    fmul(curr);
                    fadd(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                }
                else if(op[0] == '/')
                {
                    // (a+bi)/(c+di) = (ac+bd)/(c^2+d^2)+(bc-ad)i/(c^2+d^2)
                    // (c^2+d^2)
                    fld_ptr_real(curr, jit_stack_curr + 1);
                    fldi(curr, 0);
                    fmul(curr);
                    fld_ptr_imag(curr, jit_stack_curr + 1);
                    fldi(curr, 0);
                    fmul(curr);
                    fadd(curr);
                    fldi(curr, 0);
                    // ac+bd
                    fld_ptr_real(curr, jit_stack_curr);
                    fld_ptr_real(curr, jit_stack_curr + 1);
                    fmul(curr);
                    fld_ptr_imag(curr, jit_stack_curr);
                    fld_ptr_imag(curr, jit_stack_curr + 1);
                    fmul(curr);
                    fadd(curr);
                    // (ac+bd)/(c^2+d^2)
                    fdivr(curr);
                    fxch(curr);
                    // bc-ad
                    fld_ptr_imag(curr, jit_stack_curr);
                    fld_ptr_real(curr, jit_stack_curr + 1);
                    fmul(curr);
                    fld_ptr_real(curr, jit_stack_curr);
                    fld_ptr_imag(curr, jit_stack_curr + 1);
                    fmul(curr);
                    fsub(curr);
                    // (bc-ad)i/(c^2+d^2)
                    fdivr(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                }
                else if(op[0] == '^')
                {
                    // pow(a, z) = r * cos(theta) + (r * sin(theta)) * I;
                    // r = pow(Abs(a), Re(z)) * exp(-Im(z) * Arg(a));
                    // theta = Re(z) * Arg(a) + Im(z) * log(Abs(a));
                    //
                    // Abs(a)
                    fld_ptr_imag(curr, jit_stack_curr);
                    fldi(curr, 0);
                    fmul(curr);
                    fld_ptr_real(curr, jit_stack_curr);
                    fldi(curr, 0);
                    fmul(curr);
                    fadd(curr);
                    fsqrt(curr);
                    fldi(curr, 0);
                    fstp_ptr_real(curr, jit_stack_curr + 3); // temp store for Abs(a)
                    //
                    // Arg(a)
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    // TODO: Hmm... Does it work if y == 0?
                    fld_ptr_real(curr, jit_stack_curr);
                    fsub(curr);
                    fld_ptr_imag(curr, jit_stack_curr);
                    fpatan(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    fstp_ptr_imag(curr, jit_stack_curr + 3); // temp store for Arg(a)
                    //
                    // log(Abs(a))
                    fld_ptr_real(curr, jit_stack_curr + 3);
                    fld1(curr);
                    fxch(curr);
                    fyl2x(curr);
                    fldl2e(curr);
                    fdiv(curr);
                    //
                    // theta = Re(z) * Arg(a) + Im(z) * log(Abs(a));
                    fld_ptr_imag(curr, jit_stack_curr + 1);
                    fmul(curr);
                    fld_ptr_real(curr, jit_stack_curr + 1);
                    fld_ptr_imag(curr, jit_stack_curr + 3);
                    fmul(curr);
                    fadd(curr);
                    fstp_ptr_real(curr, jit_stack_curr + 4); // temp store for theta
                    //
                    // pow(Abs(a), Re(z))
                    fld_ptr_real(curr, jit_stack_curr + 1);
                    fld_ptr_real(curr, jit_stack_curr + 3);
                    fyl2x(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr_real(curr, jit_stack_curr + 2); // garbage
                    fstp_ptr_imag(curr, jit_stack_curr + 4); // temp store for pow
                    //
                    // exp(-Im(z) * Arg(a));
                    fld_ptr_imag(curr, jit_stack_curr + 3);
                    fld_ptr_imag(curr, jit_stack_curr + 1);
                    fchs(curr);
                    fmul(curr);
                    fldl2e(curr);
                    fmul(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr_real(curr, jit_stack_curr + 2); // garbage
                    //
                    // r = pow(Abs(a), Re(z)) * exp(-Im(z) * Arg(a));
                    fld_ptr_imag(curr, jit_stack_curr + 4);
                    fmul(curr);
                    //
                    // pow(a, z) = r * cos(theta) + (r * sin(theta)) * I;
                    fld_ptr_real(curr, jit_stack_curr + 4);
                    fsincos(curr);
                    fldi(curr, 2);
                    fmul(curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                    fmul(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                }
                else
                {
                    error_string = "Unsupported operator " + it->str();
                    return false;
                }
                jit_stack_curr++;
            }
            else if(it->is_function())
            {
                string fu = it->str();
                jit_stack_curr--;
                if(fu == "real")
                {
                    fldz(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                }
                else if(fu == "imag")
                {
                    fld_ptr_imag(curr, jit_stack_curr);
                    fldz(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                }
                else if(fu == "conj")
                {
                    fld_ptr_imag(curr, jit_stack_curr);
                    fchs(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                }
                else if(fu == "arg")
                {
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    // TODO: Hmm... Does it work if y == 0?
                    fld_ptr_imag(curr, jit_stack_curr);
                    fldi(curr, 0);
                    fldi(curr, 0);
                    fmul(curr);
                    fld_ptr_real(curr, jit_stack_curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fldi(curr, 0);
                    fmul(curr);
                    fadd(curr);
                    fsqrt(curr);
                    fsubr(curr);
                    fxch(curr);
                    fpatan(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    fldz(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                }
                else if(fu == "abs")
                {
                    fld_ptr_imag(curr, jit_stack_curr);
                    fldi(curr, 0);
                    fmul(curr);
                    fld_ptr_real(curr, jit_stack_curr);
                    fldi(curr, 0);
                    fmul(curr);
                    fadd(curr);
                    fsqrt(curr);
                    fldz(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                }
                else if(fu == "exp")
                {
                    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
                    //
                    // exp(Re(z))
                    fld_ptr_real(curr, jit_stack_curr);
                    fldl2e(curr);
                    fmul(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr_real(curr, jit_stack_curr + 1);
                    // cos(Im(z)), sin(Im(z))
                    fld_ptr_imag(curr, jit_stack_curr);
                    fsincos(curr);
                    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
                    fldi(curr, 2);
                    fmul(curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                    fmul(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                }
                else if(fu == "sin")
                {
                    T * px = jit_stack_curr;
                    T * tmp = jit_stack_curr + 1;
                    // sin(x) = (exp(ix) - exp(-ix)) / 2i
                    //
                    // exp(ix)
                    fld_ptr_imag(curr, px);
                    fchs(curr);
                    fldl2e(curr);
                    fmul(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr_real(curr, tmp + 1);
                    // cos(Im(z)), sin(Im(z))
                    fld_ptr_real(curr, px);
                    fsincos(curr);
                    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
                    fldi(curr, 2);
                    fmul(curr);
                    fstp_ptr_real(curr, tmp);
                    fmul(curr);
                    fstp_ptr_imag(curr, tmp);
                    //
                    // exp(-ix)
                    fld_ptr_imag(curr, px);
                    fldl2e(curr);
                    fmul(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr_real(curr, tmp + 1);
                    // cos(Im(z)), sin(Im(z))
                    fld_ptr_real(curr, px);
                    fchs(curr);
                    fsincos(curr);
                    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
                    fldi(curr, 2);
                    fmul(curr);
                    fstp_ptr_real(curr, tmp + 1);
                    fmul(curr);
                    fstp_ptr_imag(curr, tmp + 1);
                    //
                    // exp(ix) - exp(-ix)
                    fld_ptr_real(curr, tmp);
                    fld_ptr_real(curr, tmp + 1);
                    fsub(curr);
                    fstp_ptr_real(curr, tmp);
                    fld_ptr_imag(curr, tmp);
                    fld_ptr_imag(curr, tmp + 1);
                    fsub(curr);
                    fstp_ptr_imag(curr, tmp);
                    //
                    // sin(x) = (exp(ix) - exp(-ix)) / 2i
                    // (a+bi) / 2i = (b-ia) * 0.5 = 0.5b - 0.5a
                    fld1(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fdiv(curr);
                    fldi(curr, 0);
                    fld_ptr_imag(curr, tmp);
                    fmul(curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                    fld_ptr_real(curr, tmp);
                    fmul(curr);
                    fchs(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                }
                else if(fu == "cos")
                {
                    T * px = jit_stack_curr;
                    T * tmp = jit_stack_curr + 1;
                    // cos(x) = (exp(ix) + exp(-ix)) / 2
                    //
                    // exp(ix)
                    fld_ptr_imag(curr, px);
                    fchs(curr);
                    fldl2e(curr);
                    fmul(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr_real(curr, tmp + 1);
                    // cos(Im(z)), sin(Im(z))
                    fld_ptr_real(curr, px);
                    fsincos(curr);
                    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
                    fldi(curr, 2);
                    fmul(curr);
                    fstp_ptr_real(curr, tmp);
                    fmul(curr);
                    fstp_ptr_imag(curr, tmp);
                    //
                    // exp(-ix)
                    fld_ptr_imag(curr, px);
                    fldl2e(curr);
                    fmul(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr_real(curr, tmp + 1);
                    // cos(Im(z)), sin(Im(z))
                    fld_ptr_real(curr, px);
                    fchs(curr);
                    fsincos(curr);
                    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
                    fldi(curr, 2);
                    fmul(curr);
                    fstp_ptr_real(curr, tmp + 1);
                    fmul(curr);
                    fstp_ptr_imag(curr, tmp + 1);
                    //
                    // exp(ix) + exp(-ix)
                    fld_ptr_real(curr, tmp);
                    fld_ptr_real(curr, tmp + 1);
                    fadd(curr);
                    fstp_ptr_real(curr, tmp);
                    fld_ptr_imag(curr, tmp);
                    fld_ptr_imag(curr, tmp + 1);
                    fadd(curr);
                    fstp_ptr_imag(curr, tmp);
                    //
                    // cos(x) = (exp(ix) + exp(-ix)) * 0.5
                    // (a+bi)*(c) = ac+bci
                    fld1(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fdiv(curr);
                    fldi(curr, 0);
                    fld_ptr_real(curr, tmp);
                    fmul(curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                    fld_ptr_imag(curr, tmp);
                    fmul(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                }
                else if(fu == "tan")
                {
                    T * px = jit_stack_curr;
                    T * tmp = jit_stack_curr + 1;
                    // tan(x) = (exp(ix) - exp(-ix)) / (i * (exp(ix) + exp(-ix)))
                    //
                    // exp(ix)
                    fld_ptr_imag(curr, px);
                    fchs(curr);
                    fldl2e(curr);
                    fmul(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr_real(curr, tmp + 1);
                    // cos(Im(z)), sin(Im(z))
                    fld_ptr_real(curr, px);
                    fsincos(curr);
                    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
                    fldi(curr, 2);
                    fmul(curr);
                    fstp_ptr_real(curr, tmp);
                    fmul(curr);
                    fstp_ptr_imag(curr, tmp);
                    //
                    // exp(-ix)
                    fld_ptr_imag(curr, px);
                    fldl2e(curr);
                    fmul(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    fsub(curr);
                    f2xm1(curr);
                    fld1(curr);
                    fadd(curr);
                    fscale(curr);
                    fxch(curr);
                    fstp_ptr_real(curr, tmp + 1);
                    // cos(Im(z)), sin(Im(z))
                    fld_ptr_real(curr, px);
                    fchs(curr);
                    fsincos(curr);
                    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
                    fldi(curr, 2);
                    fmul(curr);
                    fstp_ptr_real(curr, tmp + 1);
                    fmul(curr);
                    fstp_ptr_imag(curr, tmp + 1);
                    //
                    // exp(ix) - exp(-ix)
                    fld_ptr_real(curr, tmp);
                    fld_ptr_real(curr, tmp + 1);
                    fsub(curr);
                    fstp_ptr_real(curr, tmp + 2);
                    fld_ptr_imag(curr, tmp);
                    fld_ptr_imag(curr, tmp + 1);
                    fsub(curr);
                    fstp_ptr_imag(curr, tmp + 2);
                    //
                    // i * (exp(ix) + exp(-ix))
                    fld_ptr_real(curr, tmp);
                    fld_ptr_real(curr, tmp + 1);
                    fadd(curr);
                    fstp_ptr_imag(curr, tmp + 3);
                    fld_ptr_imag(curr, tmp);
                    fld_ptr_imag(curr, tmp + 1);
                    fadd(curr);
                    fchs(curr);
                    fstp_ptr_real(curr, tmp + 3);
                    //
                    // tan(x) = (exp(ix) - exp(-ix)) / (i * (exp(ix) + exp(-ix)))
                    // (a+bi)/(c+di) = (ac+bd)/(c^2+d^2)+(bc-ad)i/(c^2+d^2)
                    // (c^2+d^2)
                    fld_ptr_real(curr, tmp + 3);
                    fldi(curr, 0);
                    fmul(curr);
                    fld_ptr_imag(curr, tmp + 3);
                    fldi(curr, 0);
                    fmul(curr);
                    fadd(curr);
                    fldi(curr, 0);
                    // ac+bd
                    fld_ptr_real(curr, tmp + 2);
                    fld_ptr_real(curr, tmp + 3);
                    fmul(curr);
                    fld_ptr_imag(curr, tmp + 2);
                    fld_ptr_imag(curr, tmp + 3);
                    fmul(curr);
                    fadd(curr);
                    // (ac+bd)/(c^2+d^2)
                    fdivr(curr);
                    fxch(curr);
                    // bc-ad
                    fld_ptr_imag(curr, tmp + 2);
                    fld_ptr_real(curr, tmp + 3);
                    fmul(curr);
                    fld_ptr_real(curr, tmp + 2);
                    fld_ptr_imag(curr, tmp + 3);
                    fmul(curr);
                    fsub(curr);
                    // (bc-ad)i/(c^2+d^2)
                    fdivr(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                }
                else if(fu == "log" || fu == "log2" || fu == "log10")
                {
                    // log(z) = ln(abs(z))+i*arg(z)
                    //
                    // abs(z)
                    fld_ptr_imag(curr, jit_stack_curr);
                    fldi(curr, 0);
                    fmul(curr);
                    fld_ptr_real(curr, jit_stack_curr);
                    fldi(curr, 0);
                    fmul(curr);
                    fadd(curr);
                    fsqrt(curr);
                    fldi(curr, 0);
                    //
                    // ln(abs(z))
                    fld1(curr);
                    fxch(curr);
                    fyl2x(curr);
                    fldl2e(curr);
                    fdiv(curr);
                    fxch(curr);
                    //
                    // arg(z)
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    // TODO: Hmm... Does it work if y == 0?
                    fld_ptr_real(curr, jit_stack_curr);
                    fsub(curr);
                    fld_ptr_imag(curr, jit_stack_curr);
                    fpatan(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    //
                    if(fu == "log")
                    {
                        fstp_ptr_imag(curr, jit_stack_curr);
                        fstp_ptr_real(curr, jit_stack_curr);
                    }
                    else
                    {
                        if(fu == "log2")
                        {
                            fld1(curr);
                            fldln2(curr);
                        }
                        else
                        {
                            fldl2e(curr);
                            fldl2t(curr);
                        }
                        fdiv(curr);
                        fxch(curr);
                        fldi(curr, 1);
                        // (a+bi)*c = (ac)+(bc)i
                        fmul(curr);
                        fstp_ptr_imag(curr, jit_stack_curr);
                        fmul(curr);
                        fstp_ptr_real(curr, jit_stack_curr);
                    }
                }
                else if(fu == "sqrt")
                {
                    // sqrt(z) = sqrt((|z| + Re(z))/2) + i * sign(Im(z)) * sqrt((|z| - Re(z))/2)
                    //
                    // sign(Im(z))
                    // TODO: EXTREMELY DANGEROUS MAGIC!
                    fld_ptr_imag(curr, jit_stack_curr);
                    fldi(curr, 0);
                    fabs(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fdiv(curr);
                    fsub(curr);
                    fxtract(curr);
                    fxch(curr);
                    fstp_ptr_real(curr, jit_stack_curr + 1);
                    fldpi(curr);
                    fdiv(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    frndint(curr);
                    //
                    // |z|
                    fld_ptr_imag(curr, jit_stack_curr);
                    fldi(curr, 0);
                    fmul(curr);
                    fld_ptr_real(curr, jit_stack_curr);
                    fldi(curr, 0);
                    fmul(curr);
                    fadd(curr);
                    fsqrt(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    //
                    // sqrt((|z| - Re(z))/2)
                    fld_ptr_real(curr, jit_stack_curr);
                    fsub(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fdiv(curr);
                    fsqrt(curr);
                    //
                    // sign(Im(z)) * sqrt((|z| - Re(z))/2)
                    fmul(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                    //
                    // sqrt((|z| + Re(z))/2)
                    fld_ptr_real(curr, jit_stack_curr);
                    fadd(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fdiv(curr);
                    fsqrt(curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                }
                else if(fu == "asin" || fu == "acos")
                {
                    T * z = jit_stack_curr;
                    T * tmp = jit_stack_curr + 1;
                    // asin(z) = - i * log(i * z + sqrt((T)1 - z * z))
                    // acos(z) = pi/2 - asin(z)
                    //
                    // 1 - z * z
                    // 1-(a+bi)^2 = (1-aa+bb)-2abi
                    fld_ptr_real(curr, z);
                    fld_ptr_imag(curr, z);
                    fldi(curr, 1);
                    fldi(curr, 0);
                    fmul(curr);
                    fld1(curr);
                    fsubr(curr);
                    fldi(curr, 1);
                    fldi(curr, 0);
                    fmul(curr);
                    fxch(curr);
                    fadd(curr);
                    fstp_ptr_real(curr, tmp);
                    fmul(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    fchs(curr);
                    fstp_ptr_imag(curr, tmp);
                    //
                    // sqrt((T)1 - z * z)
                    // sqrt(z) = sqrt((|z| + Re(z))/2) + i * sign(Im(z)) * sqrt((|z| - Re(z))/2)
                    //
                    // sign(Im(z))
                    // TODO: EXTREMELY DANGEROUS MAGIC!
                    fld_ptr_imag(curr, tmp);
                    fldi(curr, 0);
                    fabs(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fdiv(curr);
                    fsub(curr);
                    fxtract(curr);
                    fxch(curr);
                    fstp_ptr_real(curr, tmp + 1);
                    fldpi(curr);
                    fdiv(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    frndint(curr);
                    //
                    // |z|
                    fld_ptr_imag(curr, tmp);
                    fldi(curr, 0);
                    fmul(curr);
                    fld_ptr_real(curr, tmp);
                    fldi(curr, 0);
                    fmul(curr);
                    fadd(curr);
                    fsqrt(curr);
                    fxch(curr);
                    fldi(curr, 1);
                    //
                    // sqrt((|z| - Re(z))/2)
                    fld_ptr_real(curr, tmp);
                    fsub(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fdiv(curr);
                    fsqrt(curr);
                    //
                    // sign(Im(z)) * sqrt((|z| - Re(z))/2)
                    fmul(curr);
                    fstp_ptr_imag(curr, tmp);
                    //
                    // sqrt((|z| + Re(z))/2)
                    fld_ptr_real(curr, tmp);
                    fadd(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fdiv(curr);
                    fsqrt(curr);
                    //fstp_ptr_real(curr, tmp);
                    //
                    // i * z + sqrt((T)1 - z * z)
                    //fld_ptr_real(curr, tmp);
                    fld_ptr_imag(curr, z);
                    fsub(curr);
                    fld_ptr_real(curr, z);
                    fld_ptr_imag(curr, tmp);
                    fadd(curr);
                    fstp_ptr_imag(curr, tmp);
                    fstp_ptr_real(curr, tmp);
                    //
                    // log(i * z + sqrt((T)1 - z * z))
                    // log(z) = ln(abs(z))+i*arg(z)
                    //
                    // abs(z)
                    fld_ptr_imag(curr, tmp);
                    fldi(curr, 0);
                    fmul(curr);
                    fld_ptr_real(curr, tmp);
                    fldi(curr, 0);
                    fmul(curr);
                    fadd(curr);
                    fsqrt(curr);
                    fldi(curr, 0);
                    //
                    // ln(abs(z))
                    fld1(curr);
                    fxch(curr);
                    fyl2x(curr);
                    fldl2e(curr);
                    fdiv(curr);
                    fxch(curr);
                    //
                    // arg(z)
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    // TODO: Hmm... Does it work if y == 0?
                    fld_ptr_real(curr, tmp);
                    fsub(curr);
                    fld_ptr_imag(curr, tmp);
                    fpatan(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    //
                    if(fu == "asin")
                    {
                        // asin(z) = - i * log(i * z + sqrt((T)1 - z * z))
                        fstp_ptr_real(curr, jit_stack_curr);
                        fchs(curr);
                        fstp_ptr_imag(curr, jit_stack_curr);
                    }
                    else
                    {
                        // acos(z) = pi/2 - asin(z)
                        fldpi(curr);
                        fld1(curr);
                        fld1(curr);
                        fadd(curr);
                        fdiv(curr);
                        fsubr(curr);
                        fstp_ptr_real(curr, jit_stack_curr);
                        fstp_ptr_imag(curr, jit_stack_curr);
                    }
                }
                else if(fu == "atan")
                {
                    T * z = jit_stack_curr;
                    T * l1 = jit_stack_curr + 1;
                    T * l2 = jit_stack_curr + 2;
                    // atan(z) = i/2 * (log(1-zi)-log(zi+1));
                    //
                    // 1-zi, 1+zi
                    fld_ptr_imag(curr, z);
                    fldi(curr, 0);
                    fld1(curr);
                    fadd(curr);
                    fstp_ptr_real(curr, l1);
                    fld1(curr);
                    fsubr(curr);
                    fstp_ptr_real(curr, l2);
                    fld_ptr_real(curr, z);
                    fldi(curr, 0);
                    fchs(curr);
                    fstp_ptr_imag(curr, l1);
                    fstp_ptr_imag(curr, l2);
                    //
                    // log(1-zi)
                    // log(z) = ln(abs(z))+i*arg(z)
                    //
                    // abs(z)
                    fld_ptr_imag(curr, l1);
                    fldi(curr, 0);
                    fmul(curr);
                    fld_ptr_real(curr, l1);
                    fldi(curr, 0);
                    fmul(curr);
                    fadd(curr);
                    fsqrt(curr);
                    fldi(curr, 0);
                    //
                    // ln(abs(z))
                    fld1(curr);
                    fxch(curr);
                    fyl2x(curr);
                    fldl2e(curr);
                    fdiv(curr);
                    fxch(curr);
                    //
                    // arg(z)
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    // TODO: Hmm... Does it work if y == 0?
                    fld_ptr_real(curr, l1);
                    fsub(curr);
                    fld_ptr_imag(curr, l1);
                    fpatan(curr);
                    //fld1(curr);
                    //fld1(curr);
                    //fadd(curr);
                    //fmul(curr);
                    //
                    fstp_ptr_imag(curr, l1);
                    fstp_ptr_real(curr, l1);
                    //
                    // log(zi+1))
                    // log(z) = ln(abs(z))+i*arg(z)
                    //
                    // abs(z)
                    fld_ptr_imag(curr, l2);
                    fldi(curr, 0);
                    fmul(curr);
                    fld_ptr_real(curr, l2);
                    fldi(curr, 0);
                    fmul(curr);
                    fadd(curr);
                    fsqrt(curr);
                    fldi(curr, 0);
                    //
                    // ln(abs(z))
                    fld1(curr);
                    fxch(curr);
                    fyl2x(curr);
                    fldl2e(curr);
                    fdiv(curr);
                    fxch(curr);
                    //
                    // arg(z)
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    // TODO: Hmm... Does it work if y == 0?
                    fld_ptr_real(curr, l2);
                    fsub(curr);
                    fld_ptr_imag(curr, l2);
                    fpatan(curr);
                    //fld1(curr);
                    //fld1(curr);
                    //fadd(curr);
                    //fmul(curr);
                    //
                    fstp_ptr_imag(curr, l2);
                    fstp_ptr_real(curr, l2);
                    //
                    // atan(z) = (i * (log(1-zi)-log(zi+1))) * 0.5;
                    // (a+bi)*(c) = (ac)+(bc)i
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fld1(curr);
                    fdivr(curr);
                    fld_ptr_imag(curr, l2);
                    fld_ptr_imag(curr, l1);
                    fsub(curr);
                    //fldi(curr, 1);
                    //fmul(curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                    fld_ptr_real(curr, l1);
                    fld_ptr_real(curr, l2);
                    fsub(curr);
                    fmul(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                }
                // TODO: atan sinh cosh tanh asinh acosh atanh
                else
                {
                    error_string = "Unsupported function " + it->str();
                    return false;
                }
                jit_stack_curr++;
            }
        }

        jit_stack_curr--;
    }
    else
    {
        error_string = "Unsupported type " + string(typeid(T).name()) + "!";
        return false;
    }

    ret(curr);

#else
    error_string = "Unsupported arch!";
    return false;
#endif

    if(jit_stack_curr != jit_stack)
    {
        stringstream sst;
        sst << "Stack size equal " << (size_t)(jit_stack_curr - jit_stack);
        error_string = sst.str();
        return false;
    }

    is_compiled = true;
    return true;
#else
    error_string = "JIT is disabled!";
    return false;
#endif
}

#endif // PARSER_COMPILER_INLINE_H

