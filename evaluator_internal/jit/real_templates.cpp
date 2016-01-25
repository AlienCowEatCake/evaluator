#include "real_templates.h"

namespace evaluator_internal_jit
{

// Input:  st(0) = X , st(1) = Y
// Output: st(0) = X ^ Y
void real_pow(char *& code_curr)
{
    push_ebx(code_curr);
    // http://wasm.ru/public/forum/viewtopic.php?pid=79346#p79346
    ftst(code_curr);
    fstsw_ax(code_curr);
    sahf(code_curr);
    char * jump_pow_1 = code_curr;
    jz(code_curr, NULL); // pow_zero
    mov_bl_ah(code_curr);
    char * jump_pow_2 = code_curr;
    ja(code_curr, NULL); // pow_positive
    fxch(code_curr);
    fldi(code_curr, 0);
    frndint(code_curr);
    fcomp(code_curr);
    fstsw_ax(code_curr);
    sahf(code_curr);
    char * jump_pow_3 = code_curr;
    jnz(code_curr, NULL); // pow_error
    fld2(code_curr);
    fldi(code_curr, 1);
    fprem(code_curr);
    ftst(code_curr);
    fstsw_ax(code_curr);
    fstpi(code_curr, 0);
    fstpi(code_curr, 0);
    fxch(code_curr);
    ja(jump_pow_2, code_curr); // pow_positive
    fabs(code_curr);
    // https://stackoverflow.com/questions/4638473/how-to-powreal-real-in-x86
    fyl2x(code_curr);
    fldi(code_curr, 0);
    frndint(code_curr);
    fxch(code_curr);
    fldi(code_curr, 1);
    fsub(code_curr);
    f2xm1(code_curr);
    fld1(code_curr);
    fadd(code_curr);
    fscale(code_curr);
    fstpi(code_curr, 1);
    //
    test_bl_1(code_curr);
    char * jump_pow_4 = code_curr;
    jz(code_curr, NULL); // pow_end
    sahf(code_curr);
    char * jump_pow_5 = code_curr;
    jz(code_curr, NULL); // pow_end
    fchs(code_curr);
    char * jump_pow_6 = code_curr;
    jmp(code_curr, NULL); // pow_end
    jnz(jump_pow_3, code_curr); // pow_error
    fldz(code_curr);
    fstpi(code_curr, 1);
    stc(code_curr);
    jz(jump_pow_1, code_curr); // pow_zero
    fstpi(code_curr, 1);
    jz(jump_pow_4, code_curr); // pow_end
    jz(jump_pow_5, code_curr); // pow_end
    jmp(jump_pow_6, code_curr); // pow_end
    //
    pop_ebx(code_curr);
}

// Input:  st(0) = X
// Output: st(0) = asin(X)
void real_asin(char *& code_curr)
{
    // arcsin(a) = arctg(a / sqrt(1 - a^2))
    fldi(code_curr, 0);
    fldi(code_curr, 0);
    fmul(code_curr);
    fld1(code_curr);
    fsubr(code_curr);
    fsqrt(code_curr);
    fpatan(code_curr);
}

// Input:  st(0) = X
// Output: st(0) = acos(X)
void real_acos(char *& code_curr)
{
    // arccos(a) = 2 * arctg(sqrt(1 - a) / sqrt(1 + a))
    fldi(code_curr, 0);
    fld1(code_curr);
    fsubr(code_curr);
    fsqrt(code_curr);
    fxch(code_curr);
    fld1(code_curr);
    fadd(code_curr);
    fsqrt(code_curr);
    fpatan(code_curr);
    fld2(code_curr);
    fmul(code_curr);
}

// Input:  st(0) = X
// Output: st(0) = log2(X)
void real_log2(char *& code_curr)
{
    fld1(code_curr);
    fxch(code_curr);
    fyl2x(code_curr);
}

// Input:  st(0) = X
// Output: st(0) = log(X)
void real_log(char *& code_curr)
{
    real_log2(code_curr);
    fldl2e(code_curr);
    fdiv(code_curr);
}

// Input:  st(0) = X
// Output: st(0) = log10(X)
void real_log10(char *& code_curr)
{
    real_log2(code_curr);
    fldl2t(code_curr);
    fdiv(code_curr);
}

// Input:  st(0) = X
// Output: st(0) = exp(X)
void real_exp(char *& code_curr)
{
    // http://mathforum.org/kb/message.jspa?messageID=1640026
    fldl2e(code_curr);
    fmul(code_curr);
    fldi(code_curr, 0);
    frndint(code_curr);
    fxch(code_curr);
    fldi(code_curr, 1);
    fsub(code_curr);
    f2xm1(code_curr);
    fld1(code_curr);
    fadd(code_curr);
    fscale(code_curr);
    fstpi(code_curr, 1);
}

// Input:  st(0) = X
// Output: st(0) = sinh(X) if type == 's' ,
//                 cosh(X) if type == 'c'
static void real_helper_sinh_cosh(char *& code_curr, char type)
{
    // sinh(x) = (exp(x) - exp(-x)) / 2
    // cosh(x) = (exp(x) + exp(-x)) / 2
    fldi(code_curr, 0);
    // exp(x)
    real_exp(code_curr);
    // exp(-x)
    fxch(code_curr);
    fchs(code_curr);
    real_exp(code_curr);
    // (exp(x) +- exp(-x)) / 2
    if(type == 'c')
        fadd(code_curr);
    else
        fsub(code_curr);
    fld2(code_curr);
    fdiv(code_curr);
}

// Input:  st(0) = X
// Output: st(0) = sinh(X)
void real_sinh(char *& code_curr)
{
    real_helper_sinh_cosh(code_curr, 's');
}

// Input:  st(0) = X
// Output: st(0) = cosh(X)
void real_cosh(char *& code_curr)
{
    real_helper_sinh_cosh(code_curr, 'c');
}

// Input:  st(0) = X
// Output: st(0) = tanh(X)
void real_tanh(char *& code_curr)
{
    // tanh(x) = (exp(2*x) - 1) / (exp(2*x) + 1)
    // 2*x
    fld2(code_curr);
    fmul(code_curr);
    // exp(2*x)
    real_exp(code_curr);
    // exp(2*x) - 1
    fldi(code_curr, 0);
    fld1(code_curr);
    fsub(code_curr);
    // exp(2*x) + 1
    fxch(code_curr);
    fld1(code_curr);
    fadd(code_curr);
    // (exp(2*x) - 1) / (exp(2*x) + 1)
    fdiv(code_curr);
}

// Input:  st(0) = X
// Output: st(0) = asinh(X) if type == 's' ,
//                 acosh(X) if type == 'c'
static void real_helper_asinh_acosh(char *& code_curr, char type)
{
    // asinh(x) = log(x + sqrt(x * x + 1))
    // acosh(x) = log(x + sqrt(x * x - 1))
    fldi(code_curr, 0);
    fldi(code_curr, 0);
    fmul(code_curr);
    fld1(code_curr);
    if(type == 'c')
        fsub(code_curr);
    else
        fadd(code_curr);
    fsqrt(code_curr);
    fadd(code_curr);
    // log(...)
    real_log(code_curr);
}

// Input:  st(0) = X
// Output: st(0) = asinh(X)
void real_asinh(char *& code_curr)
{
    real_helper_asinh_acosh(code_curr, 's');
}

// Input:  st(0) = X
// Output: st(0) = acosh(X)
void real_acosh(char *& code_curr)
{
    real_helper_asinh_acosh(code_curr, 'c');
}

// Input:  st(0) = X
// Output: st(0) = atanh(X)
void real_atanh(char *& code_curr)
{
    // 0.5 * log((1.0 + x) / (1.0 - x))
    fldi(code_curr, 0);
    fld1(code_curr);
    fadd(code_curr);
    fxch(code_curr);
    fld1(code_curr);
    fsubr(code_curr);
    fdiv(code_curr);
    // log(...)
    real_log(code_curr);
    // 0.5 * log(...)
    fld1(code_curr);
    fld2(code_curr);
    fdiv(code_curr);
    fmul(code_curr);
}

// Input:  st(0) = X
// Output: st(0) = arg(X)
void real_arg(char *& code_curr)
{
    // 0 if x > 0; pi if x < 0
    fldz(code_curr);
    fcomp(code_curr);
    fstpi(code_curr, 0);
    fstsw_ax(code_curr);
    sahf(code_curr);
    char * jump1 = code_curr;
    ja(code_curr, NULL); // pi
    char * jump2 = code_curr;
    jb(code_curr, NULL); // zero
    fldz(code_curr);
    stc(code_curr);
    char * jump3 = code_curr;
    jmp(code_curr, NULL); // end
    ja(jump1, code_curr); // pi
    fldpi(code_curr);
    char * jump4 = code_curr;
    jmp(code_curr, NULL); // end
    jb(jump2, code_curr); // zero
    fldz(code_curr);
    jmp(jump3, code_curr); // end
    jmp(jump4, code_curr); // end
}

}
