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

    memset(jit_code, '\xc3', jit_code_size);
    memset(jit_stack, 0, jit_stack_size);

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
                    push_eax(curr);
                    push_ebx(curr);
                    fxch(curr);
                    // http://wasm.ru/public/forum/viewtopic.php?pid=79346#p79346
                    ftst(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_pow_1 = curr;
                    jz(curr, NULL, "pow_zero");
                    mov_bl_ah(curr);
                    char * jump_pow_2 = curr;
                    ja(curr, NULL, "pow_positive");
                    fxch(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_pow_3 = curr;
                    jnz(curr, NULL, "pow_error");
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fldi(curr, 1);
                    fprem(curr);
                    ftst(curr);
                    fstsw_ax(curr);
                    fstpi(curr, 0);
                    fstpi(curr, 0);
                    fxch(curr);
                    ja(jump_pow_2, curr, "pow_positive");
                    fabs(curr);
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
                    fstpi(curr, 1);
                    //
                    test_bl_1(curr);
                    char * jump_pow_4 = curr;
                    jz(curr, NULL, "pow_end");
                    sahf(curr);
                    char * jump_pow_5 = curr;
                    jz(curr, NULL, "pow_end");
                    fchs(curr);
                    char * jump_pow_6 = curr;
                    jmp(curr, NULL, "pow_end");
                    jnz(jump_pow_3, curr, "pow_error");
                    fldz(curr);
                    fstpi(curr, 1);
                    stc(curr);
                    jz(jump_pow_1, curr, "pow_zero");
                    fstpi(curr, 1);
                    jz(jump_pow_4, curr, "pow_end");
                    jz(jump_pow_5, curr, "pow_end");
                    jmp(jump_pow_6, curr, "pow_end");
                    //
                    pop_ebx(curr);
                    pop_eax(curr);
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
                    fstpi(curr, 1);
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
                    fstpi(curr, 1);
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
                    fstpi(curr, 1);
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
                    fstpi(curr, 1);
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
                    fldz(curr);
                    fcomp(curr);
                    fstpi(curr, 0);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump1 = curr;
                    ja(curr, NULL, "pi");
                    char * jump2 = curr;
                    jb(curr, NULL, "zero");
                    fldz(curr);
                    stc(curr);
                    char * jump3 = curr;
                    jmp(curr, NULL, "end");
                    ja(jump1, curr, "pi");
                    fldpi(curr);
                    char * jump4 = curr;
                    jmp(curr, NULL, "end");
                    jb(jump2, curr, "zero");
                    fldz(curr);
                    jmp(jump3, curr, "end");
                    jmp(jump4, curr, "end");
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
                    push_eax(curr);
                    push_ebx(curr);
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
                    fld_ptr_imag(curr, jit_stack_curr);
                    fldz(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_1 = curr;
                    jz(curr, NULL, "arg_zero_imag");
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    fxch(curr);
                    fld_ptr_real(curr, jit_stack_curr);
                    fsub(curr);
                    fxch(curr);
                    fpatan(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    char * jump_arg_2 = curr;
                    jmp(curr, NULL, "arg_end");
                    jz(jump_arg_1, curr, "arg_zero_imag");
                    fstpi(curr, 0);
                    fld_ptr_real(curr, jit_stack_curr);
                    // 0 if x > 0; pi if x < 0
                    fstpi(curr, 0);
                    fldz(curr);
                    fcomp(curr);
                    fstpi(curr, 0);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_3 = curr;
                    ja(curr, NULL, "arg_pi");
                    char * jump_arg_4 = curr;
                    jb(curr, NULL, "arg_zero");
                    fldz(curr);
                    stc(curr);
                    char * jump_arg_5 = curr;
                    jmp(curr, NULL, "arg_end");
                    ja(jump_arg_3, curr, "arg_pi");
                    fldpi(curr);
                    char * jump_arg_6 = curr;
                    jmp(curr, NULL, "arg_end");
                    jb(jump_arg_4, curr, "arg_zero");
                    fldz(curr);
                    jmp(jump_arg_2, curr, "arg_end");
                    jmp(jump_arg_5, curr, "arg_end");
                    jmp(jump_arg_6, curr, "arg_end");
                    // if arg in (-pi,pi] transform it to [0,2pi]
                    // we just calculate arg-2pi if arg > pi
                    fldpi(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_7 = curr;
                    ja(curr, NULL, "arg_end_transform");
                    fldpi(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    fsub(curr);
                    ja(jump_arg_7, curr, "arg_end_transform");
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
                    // http://wasm.ru/public/forum/viewtopic.php?pid=79346#p79346
                    ftst(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_pow_1 = curr;
                    jz(curr, NULL, "pow_zero");
                    mov_bl_ah(curr);
                    char * jump_pow_2 = curr;
                    ja(curr, NULL, "pow_positive");
                    fxch(curr);
                    fldi(curr, 0);
                    frndint(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_pow_3 = curr;
                    jnz(curr, NULL, "pow_error");
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fldi(curr, 1);
                    fprem(curr);
                    ftst(curr);
                    fstsw_ax(curr);
                    fstpi(curr, 0);
                    fstpi(curr, 0);
                    fxch(curr);
                    ja(jump_pow_2, curr, "pow_positive");
                    fabs(curr);
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
                    fstpi(curr, 1);
                    //
                    test_bl_1(curr);
                    char * jump_pow_4 = curr;
                    jz(curr, NULL, "pow_end");
                    sahf(curr);
                    char * jump_pow_5 = curr;
                    jz(curr, NULL, "pow_end");
                    fchs(curr);
                    char * jump_pow_6 = curr;
                    jmp(curr, NULL, "pow_end");
                    jnz(jump_pow_3, curr, "pow_error");
                    fldz(curr);
                    fstpi(curr, 1);
                    stc(curr);
                    jz(jump_pow_1, curr, "pow_zero");
                    fstpi(curr, 1);
                    jz(jump_pow_4, curr, "pow_end");
                    jz(jump_pow_5, curr, "pow_end");
                    jmp(jump_pow_6, curr, "pow_end");
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
                    fstpi(curr, 1); // garbage
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
                    //
                    pop_ebx(curr);
                    pop_eax(curr);
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
                    fld_ptr_imag(curr, jit_stack_curr);
                    fldz(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_1 = curr;
                    jz(curr, NULL, "arg_zero_imag");
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
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
                    char * jump_arg_2 = curr;
                    jmp(curr, NULL, "arg_end");
                    jz(jump_arg_1, curr, "arg_zero_imag");
                    fstpi(curr, 0);
                    fld_ptr_real(curr, jit_stack_curr);
                    // 0 if x > 0; pi if x < 0
                    fldz(curr);
                    fcomp(curr);
                    fstpi(curr, 0);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_3 = curr;
                    ja(curr, NULL, "arg_pi");
                    char * jump_arg_4 = curr;
                    jb(curr, NULL, "arg_zero");
                    fldz(curr);
                    stc(curr);
                    char * jump_arg_5 = curr;
                    jmp(curr, NULL, "arg_end");
                    ja(jump_arg_3, curr, "arg_pi");
                    fldpi(curr);
                    char * jump_arg_6 = curr;
                    jmp(curr, NULL, "arg_end");
                    jb(jump_arg_4, curr, "arg_zero");
                    fldz(curr);
                    jmp(jump_arg_2, curr, "arg_end");
                    jmp(jump_arg_5, curr, "arg_end");
                    jmp(jump_arg_6, curr, "arg_end");
                    // if arg in (-pi,pi] transform it to [0,2pi]
                    // we just calculate arg-2pi if arg > pi
                    fldpi(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_7 = curr;
                    ja(curr, NULL, "arg_end_transform");
                    fldpi(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    fsub(curr);
                    ja(jump_arg_7, curr, "arg_end_transform");
                    //
                    fstp_ptr_real(curr, jit_stack_curr);
                    fldz(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
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
                    fstpi(curr, 1);
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
                else if(fu == "sin" || fu == "cos")
                {
                    T * px = jit_stack_curr;
                    T * tmp = jit_stack_curr + 1;
                    // sin(x) = (exp(ix) - exp(-ix)) / 2i
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
                    fstpi(curr, 1);
                    // cos(Im(z)), sin(Im(z))
                    fld_ptr_real(curr, px);
                    fsincos(curr);
                    // -- begin magic
                    fxch(curr);    // st0 - sin, st1 - cos, st2 - num
                    fxch(curr, 2); // st0 - num, st1 - cos, st2 - sin
                    fldi(curr, 2); // st0 - sin, st1 - num, st2 - cos, st3 - sin
                    fldi(curr, 2); // st0 - cos, st1 - sin, st2 - num, st3 - cos, st4 - sin
                    // -- end magic
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
                    fstpi(curr, 1);
                    // cos(Im(z)), sin(Im(z))
                    //fld_ptr_real(curr, px);
                    //fchs(curr);
                    //fsincos(curr);
                    // -- begin magic
                    fxch(curr, 2); // st0 - sin, st1 - cos, st2 - num
                    fchs(curr);
                    fxch(curr);    // st0 - cos, st1 - sin, st2 - num
                    // -- end magic
                    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
                    fldi(curr, 2);
                    fmul(curr);
                    fstp_ptr_real(curr, tmp + 1);
                    fmul(curr);
                    fstp_ptr_imag(curr, tmp + 1);
                    //
                    if(fu == "sin")
                    {
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
                    else
                    {
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
                    fstpi(curr, 1);
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
                    fstpi(curr, 1);
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
                    fld_ptr_imag(curr, jit_stack_curr);
                    fldz(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_1 = curr;
                    jz(curr, NULL, "arg_zero_imag");
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    fxch(curr);
                    fld_ptr_real(curr, jit_stack_curr);
                    fsub(curr);
                    fxch(curr);
                    fpatan(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    char * jump_arg_2 = curr;
                    jmp(curr, NULL, "arg_end");
                    jz(jump_arg_1, curr, "arg_zero_imag");
                    fstpi(curr, 0);
                    fld_ptr_real(curr, jit_stack_curr);
                    // 0 if x > 0; pi if x < 0
                    fstpi(curr, 0);
                    fldz(curr);
                    fcomp(curr);
                    fstpi(curr, 0);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_3 = curr;
                    ja(curr, NULL, "arg_pi");
                    char * jump_arg_4 = curr;
                    jb(curr, NULL, "arg_zero");
                    fldz(curr);
                    stc(curr);
                    char * jump_arg_5 = curr;
                    jmp(curr, NULL, "arg_end");
                    ja(jump_arg_3, curr, "arg_pi");
                    fldpi(curr);
                    char * jump_arg_6 = curr;
                    jmp(curr, NULL, "arg_end");
                    jb(jump_arg_4, curr, "arg_zero");
                    fldz(curr);
                    jmp(jump_arg_2, curr, "arg_end");
                    jmp(jump_arg_5, curr, "arg_end");
                    jmp(jump_arg_6, curr, "arg_end");
                    // if arg in (-pi,pi] transform it to [0,2pi]
                    // we just calculate arg-2pi if arg > pi
                    fldpi(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_7 = curr;
                    ja(curr, NULL, "arg_end_transform");
                    fldpi(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    fsub(curr);
                    ja(jump_arg_7, curr, "arg_end_transform");
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
                    // 1 if x > 0; -1 if x < 0; 0 if x == 0
                    fld_ptr_imag(curr, jit_stack_curr);
                    fldz(curr);
                    fcomp(curr);
                    fstpi(curr, 0);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_sign_1 = curr;
                    ja(curr, NULL, "sign_m1");
                    char * jump_sign_2 = curr;
                    jb(curr, NULL, "sign_1");
                    //fldz(curr);
                    fld1(curr); // TODO: WTF?
                    char * jump_sign_3 = curr;
                    jmp(curr, NULL, "sign_end");
                    ja(jump_sign_1, curr, "sign_m1");
                    fld1(curr);
                    fchs(curr);
                    char * jump_sign_4 = curr;
                    jmp(curr, NULL, "sign_end");
                    jb(jump_sign_2, curr, "sign_1");
                    fld1(curr);
                    jmp(jump_sign_3, curr, "sign_end");
                    jmp(jump_sign_4, curr, "sign_end");
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
                    // 1 if x > 0; -1 if x < 0; 0 if x == 0
                    fld_ptr_imag(curr, tmp);
                    fldz(curr);
                    fcomp(curr);
                    fstpi(curr, 0);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_sign_1 = curr;
                    ja(curr, NULL, "sign_m1");
                    char * jump_sign_2 = curr;
                    jb(curr, NULL, "sign_1");
                    //fldz(curr);
                    fld1(curr); // TODO: WTF?
                    char * jump_sign_3 = curr;
                    jmp(curr, NULL, "sign_end");
                    ja(jump_sign_1, curr, "sign_m1");
                    fld1(curr);
                    fchs(curr);
                    char * jump_sign_4 = curr;
                    jmp(curr, NULL, "sign_end");
                    jb(jump_sign_2, curr, "sign_1");
                    fld1(curr);
                    jmp(jump_sign_3, curr, "sign_end");
                    jmp(jump_sign_4, curr, "sign_end");
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
                    fld_ptr_imag(curr, tmp);
                    fldz(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_1 = curr;
                    jz(curr, NULL, "arg_zero_imag");
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    fxch(curr);
                    fld_ptr_real(curr, tmp);
                    fsub(curr);
                    fxch(curr);
                    fpatan(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    char * jump_arg_2 = curr;
                    jmp(curr, NULL, "arg_end");
                    jz(jump_arg_1, curr, "arg_zero_imag");
                    fstpi(curr, 0);
                    fld_ptr_real(curr, tmp);
                    // 0 if x > 0; pi if x < 0
                    fstpi(curr, 0);
                    fldz(curr);
                    fcomp(curr);
                    fstpi(curr, 0);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_3 = curr;
                    ja(curr, NULL, "arg_pi");
                    char * jump_arg_4 = curr;
                    jb(curr, NULL, "arg_zero");
                    fldz(curr);
                    stc(curr);
                    char * jump_arg_5 = curr;
                    jmp(curr, NULL, "arg_end");
                    ja(jump_arg_3, curr, "arg_pi");
                    fldpi(curr);
                    char * jump_arg_6 = curr;
                    jmp(curr, NULL, "arg_end");
                    jb(jump_arg_4, curr, "arg_zero");
                    fldz(curr);
                    jmp(jump_arg_2, curr, "arg_end");
                    jmp(jump_arg_5, curr, "arg_end");
                    jmp(jump_arg_6, curr, "arg_end");
                    // if arg in (-pi,pi] transform it to [0,2pi]
                    // we just calculate arg-2pi if arg > pi
                    fldpi(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_7 = curr;
                    ja(curr, NULL, "arg_end_transform");
                    fldpi(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    fsub(curr);
                    ja(jump_arg_7, curr, "arg_end_transform");
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
                    fld_ptr_imag(curr, l1);
                    fldz(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_1 = curr;
                    jz(curr, NULL, "arg_zero_imag");
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    fxch(curr);
                    fld_ptr_real(curr, l1);
                    fsub(curr);
                    fxch(curr);
                    fpatan(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    char * jump_arg_2 = curr;
                    jmp(curr, NULL, "arg_end");
                    jz(jump_arg_1, curr, "arg_zero_imag");
                    fstpi(curr, 0);
                    fld_ptr_real(curr, l1);
                    // 0 if x > 0; pi if x < 0
                    fstpi(curr, 0);
                    fldz(curr);
                    fcomp(curr);
                    fstpi(curr, 0);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_3 = curr;
                    ja(curr, NULL, "arg_pi");
                    char * jump_arg_4 = curr;
                    jb(curr, NULL, "arg_zero");
                    fldz(curr);
                    stc(curr);
                    char * jump_arg_5 = curr;
                    jmp(curr, NULL, "arg_end");
                    ja(jump_arg_3, curr, "arg_pi");
                    fldpi(curr);
                    char * jump_arg_6 = curr;
                    jmp(curr, NULL, "arg_end");
                    jb(jump_arg_4, curr, "arg_zero");
                    fldz(curr);
                    jmp(jump_arg_2, curr, "arg_end");
                    jmp(jump_arg_5, curr, "arg_end");
                    jmp(jump_arg_6, curr, "arg_end");
                    // if arg in (-pi,pi] transform it to [0,2pi]
                    // we just calculate arg-2pi if arg > pi
                    fldpi(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_7 = curr;
                    ja(curr, NULL, "arg_end_transform");
                    fldpi(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    fsub(curr);
                    ja(jump_arg_7, curr, "arg_end_transform");
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
                    fld_ptr_imag(curr, l2);
                    fldz(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    jump_arg_1 = curr;
                    jz(curr, NULL, "arg_zero_imag");
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    fxch(curr);
                    fld_ptr_real(curr, l2);
                    fsub(curr);
                    fxch(curr);
                    fpatan(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    jump_arg_2 = curr;
                    jmp(curr, NULL, "arg_end");
                    jz(jump_arg_1, curr, "arg_zero_imag");
                    fstpi(curr, 0);
                    fld_ptr_real(curr, l2);
                    // 0 if x > 0; pi if x < 0
                    fstpi(curr, 0);
                    fldz(curr);
                    fcomp(curr);
                    fstpi(curr, 0);
                    fstsw_ax(curr);
                    sahf(curr);
                    jump_arg_3 = curr;
                    ja(curr, NULL, "arg_pi");
                    jump_arg_4 = curr;
                    jb(curr, NULL, "arg_zero");
                    fldz(curr);
                    stc(curr);
                    jump_arg_5 = curr;
                    jmp(curr, NULL, "arg_end");
                    ja(jump_arg_3, curr, "arg_pi");
                    fldpi(curr);
                    jump_arg_6 = curr;
                    jmp(curr, NULL, "arg_end");
                    jb(jump_arg_4, curr, "arg_zero");
                    fldz(curr);
                    jmp(jump_arg_2, curr, "arg_end");
                    jmp(jump_arg_5, curr, "arg_end");
                    jmp(jump_arg_6, curr, "arg_end");
                    // if arg in (-pi,pi] transform it to [0,2pi]
                    // we just calculate arg-2pi if arg > pi
                    fldpi(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    jump_arg_7 = curr;
                    ja(curr, NULL, "arg_end_transform");
                    fldpi(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    fsub(curr);
                    ja(jump_arg_7, curr, "arg_end_transform");
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
                    fldi(curr, 1);
                    fmul(curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                    fld_ptr_real(curr, l1);
                    fld_ptr_real(curr, l2);
                    fsub(curr);
                    fmul(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                }
                else if(fu == "sinh" || fu == "cosh" || fu == "tanh")
                {
                    T * px = jit_stack_curr;
                    T * tmp = jit_stack_curr + 1;
                    // sinh(z) = (exp(z) - exp(-z)) / 2
                    // cosh(z) = (exp(z) + exp(-z)) / 2
                    // tanh(z) = (exp(z) - exp(-z)) / (exp(z) + exp(-z))
                    //
                    // exp(x)
                    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
                    fld_ptr_real(curr, px);
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
                    fstpi(curr, 1);
                    // cos(Im(z)), sin(Im(z))
                    fld_ptr_imag(curr, px);
                    fsincos(curr);
                    // -- begin magic
                    fxch(curr);    // st0 - sin, st1 - cos, st2 - num
                    fxch(curr, 2); // st0 - num, st1 - cos, st2 - sin
                    fldi(curr, 2); // st0 - sin, st1 - num, st2 - cos, st3 - sin
                    fldi(curr, 2); // st0 - cos, st1 - sin, st2 - num, st3 - cos, st4 - sin
                    // -- end magic
                    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
                    fldi(curr, 2);
                    fmul(curr);
                    fstp_ptr_real(curr, tmp);
                    fmul(curr);
                    fstp_ptr_imag(curr, tmp);
                    //
                    // exp(-x)
                    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
                    fld_ptr_real(curr, px);
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
                    fstpi(curr, 1);
                    // cos(Im(z)), sin(Im(z))
                    //fld_ptr_real(curr, px);
                    //fchs(curr);
                    //fsincos(curr);
                    // -- begin magic
                    fxch(curr, 2); // st0 - sin, st1 - cos, st2 - num
                    fchs(curr);
                    fxch(curr);    // st0 - cos, st1 - sin, st2 - num
                    // -- end magic
                    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
                    fldi(curr, 2);
                    fmul(curr);
                    fstp_ptr_real(curr, tmp + 1);
                    fmul(curr);
                    fstp_ptr_imag(curr, tmp + 1);
                    //
                    if(fu == "sinh" || fu == "cosh")
                    {
                        if(fu == "sinh")
                        {
                            // exp(x) - exp(-x)
                            fld_ptr_real(curr, tmp);
                            fld_ptr_real(curr, tmp + 1);
                            fsub(curr);
                            fstp_ptr_real(curr, tmp);
                            fld_ptr_imag(curr, tmp);
                            fld_ptr_imag(curr, tmp + 1);
                            fsub(curr);
                            fstp_ptr_imag(curr, tmp);
                        }
                        else
                        {
                            // exp(x) + exp(-x)
                            fld_ptr_real(curr, tmp);
                            fld_ptr_real(curr, tmp + 1);
                            fadd(curr);
                            fstp_ptr_real(curr, tmp);
                            fld_ptr_imag(curr, tmp);
                            fld_ptr_imag(curr, tmp + 1);
                            fadd(curr);
                            fstp_ptr_imag(curr, tmp);
                        }
                        //
                        // sinh(z) = (exp(z) - exp(-z)) * 0.5
                        // cosh(z) = (exp(z) + exp(-z)) * 0.5
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
                    else
                    {
                        // exp(x) - exp(-x), exp(x) + exp(-x)
                        fld_ptr_real(curr, tmp);
                        fld_ptr_real(curr, tmp + 1);
                        fldi(curr, 1);
                        fldi(curr, 1);
                        fsub(curr);
                        fstp_ptr_real(curr, tmp);
                        fadd(curr);
                        fstp_ptr_real(curr, tmp + 1);
                        fld_ptr_imag(curr, tmp);
                        fld_ptr_imag(curr, tmp + 1);
                        fldi(curr, 1);
                        fldi(curr, 1);
                        fsub(curr);
                        fstp_ptr_imag(curr, tmp);
                        fadd(curr);
                        fstp_ptr_imag(curr, tmp + 1);
                        //
                        // tanh(z) = (exp(z) - exp(-z)) / (exp(z) + exp(-z))
                        // (a+bi)/(c+di) = (ac+bd)/(c^2+d^2)+(bc-ad)i/(c^2+d^2)
                        // (c^2+d^2)
                        fld_ptr_real(curr, tmp + 1);
                        fldi(curr, 0);
                        fmul(curr);
                        fld_ptr_imag(curr, tmp + 1);
                        fldi(curr, 0);
                        fmul(curr);
                        fadd(curr);
                        fldi(curr, 0);
                        // ac+bd
                        fld_ptr_real(curr, tmp);
                        fld_ptr_real(curr, tmp + 1);
                        fmul(curr);
                        fld_ptr_imag(curr, tmp);
                        fld_ptr_imag(curr, tmp + 1);
                        fmul(curr);
                        fadd(curr);
                        // (ac+bd)/(c^2+d^2)
                        fdivr(curr);
                        fxch(curr);
                        // bc-ad
                        fld_ptr_imag(curr, tmp);
                        fld_ptr_real(curr, tmp + 1);
                        fmul(curr);
                        fld_ptr_real(curr, tmp);
                        fld_ptr_imag(curr, tmp + 1);
                        fmul(curr);
                        fsub(curr);
                        // (bc-ad)i/(c^2+d^2)
                        fdivr(curr);
                        fstp_ptr_imag(curr, jit_stack_curr);
                        fstp_ptr_real(curr, jit_stack_curr);
                    }
                }
                else if(fu == "asinh" || fu == "acosh")
                {
                    T * x = jit_stack_curr;
                    T * tmp = jit_stack_curr + 1;
                    // asinh(x) = log(x + sqrt(x * x + 1))
                    // acosh(x) = log(x + sqrt(x * x - 1))
                    //
                    // x * x +- 1
                    // (a+bi)^2 = (aa-bb+-1)+2abi
                    fld_ptr_real(curr, x);
                    fld_ptr_imag(curr, x);
                    fldi(curr, 1);
                    fldi(curr, 0);
                    fmul(curr);
                    fldi(curr, 1);
                    fldi(curr, 0);
                    fmul(curr);
                    fsub(curr);
                    fld1(curr);
                    if(fu == "asinh")
                        fadd(curr);
                    else
                        fsub(curr);
                    fstp_ptr_real(curr, tmp);
                    fmul(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    fstp_ptr_imag(curr, tmp);
                    //
                    // sqrt(x * x +- 1)
                    // sqrt(z) = sqrt((|z| + Re(z))/2) + i * sign(Im(z)) * sqrt((|z| - Re(z))/2)
                    //
                    // sign(Im(z))
                    // 1 if x > 0; -1 if x < 0; 0 if x == 0
                    fld_ptr_imag(curr, tmp);
                    fldz(curr);
                    fcomp(curr);
                    fstpi(curr, 0);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_sign_1 = curr;
                    ja(curr, NULL, "sign_m1");
                    char * jump_sign_2 = curr;
                    jb(curr, NULL, "sign_1");
                    //fldz(curr);
                    fld1(curr); // TODO: WTF?
                    char * jump_sign_3 = curr;
                    jmp(curr, NULL, "sign_end");
                    ja(jump_sign_1, curr, "sign_m1");
                    fld1(curr);
                    fchs(curr);
                    char * jump_sign_4 = curr;
                    jmp(curr, NULL, "sign_end");
                    jb(jump_sign_2, curr, "sign_1");
                    fld1(curr);
                    jmp(jump_sign_3, curr, "sign_end");
                    jmp(jump_sign_4, curr, "sign_end");
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
                    // x + sqrt(x * x +- 1)
                    //fld_ptr_real(curr, tmp);
                    fld_ptr_real(curr, x);
                    fadd(curr);
                    fstp_ptr_real(curr, tmp);
                    fld_ptr_imag(curr, tmp);
                    fld_ptr_imag(curr, x);
                    fadd(curr);
                    fstp_ptr_imag(curr, tmp);
                    //
                    // log(x + sqrt(x * x +- 1))
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
                    fld_ptr_imag(curr, tmp);
                    fldz(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_1 = curr;
                    jz(curr, NULL, "arg_zero_imag");
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    fxch(curr);
                    fld_ptr_real(curr, tmp);
                    fsub(curr);
                    fxch(curr);
                    fpatan(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    char * jump_arg_2 = curr;
                    jmp(curr, NULL, "arg_end");
                    jz(jump_arg_1, curr, "arg_zero_imag");
                    fstpi(curr, 0);
                    fld_ptr_real(curr, tmp);
                    // 0 if x > 0; pi if x < 0
                    fstpi(curr, 0);
                    fldz(curr);
                    fcomp(curr);
                    fstpi(curr, 0);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_3 = curr;
                    ja(curr, NULL, "arg_pi");
                    char * jump_arg_4 = curr;
                    jb(curr, NULL, "arg_zero");
                    fldz(curr);
                    stc(curr);
                    char * jump_arg_5 = curr;
                    jmp(curr, NULL, "arg_end");
                    ja(jump_arg_3, curr, "arg_pi");
                    fldpi(curr);
                    char * jump_arg_6 = curr;
                    jmp(curr, NULL, "arg_end");
                    jb(jump_arg_4, curr, "arg_zero");
                    fldz(curr);
                    jmp(jump_arg_2, curr, "arg_end");
                    jmp(jump_arg_5, curr, "arg_end");
                    jmp(jump_arg_6, curr, "arg_end");
                    // if arg in (-pi,pi] transform it to [0,2pi]
                    // we just calculate arg-2pi if arg > pi
                    fldpi(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_7 = curr;
                    ja(curr, NULL, "arg_end_transform");
                    fldpi(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    fsub(curr);
                    ja(jump_arg_7, curr, "arg_end_transform");
                    //
                    fstp_ptr_imag(curr, jit_stack_curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                }
                else if(fu == "atanh")
                {
                    // atanh(x) = 0.5 * log((1.0 + x) / (1.0 - x))
                    //
                    // 1.0 + x, 1.0 - x
                    fld_ptr_real(curr, jit_stack_curr);
                    fldi(curr, 0);
                    fld1(curr);
                    fadd(curr);
                    fxch(curr);
                    fld1(curr);
                    fsubr(curr);
                    fstp_ptr_real(curr, jit_stack_curr + 1);
                    fstp_ptr_real(curr, jit_stack_curr);
                    fld_ptr_imag(curr, jit_stack_curr);
                    fchs(curr);
                    fstp_ptr_imag(curr, jit_stack_curr + 1);
                    //
                    // (1.0 + x) / (1.0 - x)
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
                    //
                    // log((1.0 + x) / (1.0 - x))
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
                    fld_ptr_imag(curr, jit_stack_curr);
                    fldz(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_1 = curr;
                    jz(curr, NULL, "arg_zero_imag");
                    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
                    fxch(curr);
                    fld_ptr_real(curr, jit_stack_curr);
                    fsub(curr);
                    fxch(curr);
                    fpatan(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    char * jump_arg_2 = curr;
                    jmp(curr, NULL, "arg_end");
                    jz(jump_arg_1, curr, "arg_zero_imag");
                    fstpi(curr, 0);
                    fld_ptr_real(curr, jit_stack_curr);
                    // 0 if x > 0; pi if x < 0
                    fstpi(curr, 0);
                    fldz(curr);
                    fcomp(curr);
                    fstpi(curr, 0);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_3 = curr;
                    ja(curr, NULL, "arg_pi");
                    char * jump_arg_4 = curr;
                    jb(curr, NULL, "arg_zero");
                    fldz(curr);
                    stc(curr);
                    char * jump_arg_5 = curr;
                    jmp(curr, NULL, "arg_end");
                    ja(jump_arg_3, curr, "arg_pi");
                    fldpi(curr);
                    char * jump_arg_6 = curr;
                    jmp(curr, NULL, "arg_end");
                    jb(jump_arg_4, curr, "arg_zero");
                    fldz(curr);
                    jmp(jump_arg_2, curr, "arg_end");
                    jmp(jump_arg_5, curr, "arg_end");
                    jmp(jump_arg_6, curr, "arg_end");
                    // if arg in (-pi,pi] transform it to [0,2pi]
                    // we just calculate arg-2pi if arg > pi
                    fldpi(curr);
                    fcomp(curr);
                    fstsw_ax(curr);
                    sahf(curr);
                    char * jump_arg_7 = curr;
                    ja(curr, NULL, "arg_end_transform");
                    fldpi(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fmul(curr);
                    fsub(curr);
                    ja(jump_arg_7, curr, "arg_end_transform");
                    //
                    // atanh(x) = 0.5 * log((1.0 + x) / (1.0 - x))
                    // (a+bi)*(c) = ac+bci
                    fld1(curr);
                    fld1(curr);
                    fld1(curr);
                    fadd(curr);
                    fdiv(curr);
                    fxch(curr, 2);
                    fldi(curr, 2);
                    fmul(curr);
                    fstp_ptr_real(curr, jit_stack_curr);
                    fmul(curr);
                    fstp_ptr_imag(curr, jit_stack_curr);
                }
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
        error_string = "Unsupported type `" + string(typeid(T).name()) + "`!";
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

