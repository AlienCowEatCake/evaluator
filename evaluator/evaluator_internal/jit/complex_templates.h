#ifndef EVALUATOR_COMPLEX_TEMPLATES_H
#define EVALUATOR_COMPLEX_TEMPLATES_H

#include "opcodes.h"
#include "real_templates.h"

namespace evaluator_internal_jit
{

// Input:  larg = X , rarg = Y
// Output: out = X + Y if type == '+'
//               X - Y if type == '-'
template<typename T>
void complex_helper_add_sub(char *& code_curr, const T * larg, const T * rarg, const T * out, char type)
{
    fld_ptr_real(code_curr, larg);
    fld_ptr_real(code_curr, rarg);
    if(type == '+')
        fadd(code_curr);
    else
        fsub(code_curr);
    fstp_ptr_real(code_curr, out);
    fld_ptr_imag(code_curr, larg);
    fld_ptr_imag(code_curr, rarg);
    if(type == '+')
        fadd(code_curr);
    else
        fsub(code_curr);
    fstp_ptr_imag(code_curr, out);
}

// Input:  larg = X , rarg = Y
// Output: out = X + Y
template<typename T>
void complex_add(char *& code_curr, const T * larg, const T * rarg, const T * out)
{
    complex_helper_add_sub(code_curr, larg, rarg, out, '+');
}

// Input:  larg = X , rarg = Y
// Output: out = X - Y
template<typename T>
void complex_sub(char *& code_curr, const T * larg, const T * rarg, const T * out)
{
    complex_helper_add_sub(code_curr, larg, rarg, out, '-');
}

// Input:  larg = X , rarg = Y
// Output: out = X * Y
template<typename T>
void complex_mul(char *& code_curr, const T * larg, const T * rarg, const T * out)
{
    // (a+bi)*(c+di) = (ac-bd)+(bc+ad)i
    fld_ptr_real(code_curr, larg);
    fld_ptr_real(code_curr, rarg);
    fmul(code_curr);
    fld_ptr_imag(code_curr, larg);
    fld_ptr_imag(code_curr, rarg);
    fmul(code_curr);
    fsub(code_curr);
    fld_ptr_real(code_curr, larg);
    fld_ptr_imag(code_curr, rarg);
    fmul(code_curr);
    fld_ptr_imag(code_curr, larg);
    fld_ptr_real(code_curr, rarg);
    fmul(code_curr);
    fadd(code_curr);
    fstp_ptr_imag(code_curr, out);
    fstp_ptr_real(code_curr, out);
}

// Input:  larg = X , rarg = Y
// Output: out = X / Y
template<typename T>
void complex_div(char *& code_curr, const T * larg, const T * rarg, const T * out)
{
    // (a+bi)/(c+di) = (ac+bd)/(c^2+d^2)+(bc-ad)i/(c^2+d^2)
    // (c^2+d^2)
    fld_ptr_real(code_curr, rarg);
    fldi(code_curr, 0);
    fmul(code_curr);
    fld_ptr_imag(code_curr, rarg);
    fldi(code_curr, 0);
    fmul(code_curr);
    fadd(code_curr);
    fldi(code_curr, 0);
    // ac+bd
    fld_ptr_real(code_curr, larg);
    fld_ptr_real(code_curr, rarg);
    fmul(code_curr);
    fld_ptr_imag(code_curr, larg);
    fld_ptr_imag(code_curr, rarg);
    fmul(code_curr);
    fadd(code_curr);
    // (ac+bd)/(c^2+d^2)
    fdivr(code_curr);
    fxch(code_curr);
    // bc-ad
    fld_ptr_imag(code_curr, larg);
    fld_ptr_real(code_curr, rarg);
    fmul(code_curr);
    fld_ptr_real(code_curr, larg);
    fld_ptr_imag(code_curr, rarg);
    fmul(code_curr);
    fsub(code_curr);
    // (bc-ad)i/(c^2+d^2)
    fdivr(code_curr);
    fstp_ptr_imag(code_curr, out);
    fstp_ptr_real(code_curr, out);
}

// Input:  inout = X
// Output: inout = Re(X)
template<typename T>
void complex_real(char *& code_curr, const T * inout)
{
    fldz(code_curr);
    fstp_ptr_imag(code_curr, inout);
}

// Input:  inout = X
// Output: inout = Im(X)
template<typename T>
void complex_imag(char *& code_curr, const T * inout)
{
    fld_ptr_imag(code_curr, inout);
    fldz(code_curr);
    fstp_ptr_imag(code_curr, inout);
    fstp_ptr_real(code_curr, inout);
}

// Input:  inout = X
// Output: inout = conj(X)
template<typename T>
void complex_conj(char *& code_curr, const T * inout)
{
    fld_ptr_imag(code_curr, inout);
    fchs(code_curr);
    fstp_ptr_imag(code_curr, inout);
}

// Input:  in = X
// Output: st(0) = abs(X)
template<typename T>
void complex_helper_abs(char *& code_curr, const T * in)
{
    fld_ptr_imag(code_curr, in);
    fldi(code_curr, 0);
    fmul(code_curr);
    fld_ptr_real(code_curr, in);
    fldi(code_curr, 0);
    fmul(code_curr);
    fadd(code_curr);
    fsqrt(code_curr);
}

// Input:  in = X
// Output: out = abs(X)
template<typename T>
void complex_abs(char *& code_curr, const T * in, const T * out)
{
    complex_helper_abs(code_curr, in);
    fldz(code_curr);
    fstp_ptr_imag(code_curr, out);
    fstp_ptr_real(code_curr, out);
}

// Input:  st(0) = abs(X) , in = X
// Output: st(0) = arg(X)
template<typename T>
void complex_helper_arg(char *& code_curr, const T * in)
{
    fld_ptr_imag(code_curr, in);
    fldz(code_curr);
    fcomp(code_curr);
    fstsw_ax(code_curr);
    sahf(code_curr);
    char * jump_arg_1 = code_curr;
    jz(code_curr, NULL); // arg_zero_imag
    // arg(x + iy) = 2*atan((sqrt(x^2 + y^2) - x) / y) if y != 0
    fxch(code_curr);
    fld_ptr_real(code_curr, in);
    fsub(code_curr);
    fxch(code_curr);
    fpatan(code_curr);
    fld2(code_curr);
    fmul(code_curr);
    char * jump_arg_2 = code_curr;
    jmp(code_curr, NULL); // arg_end
    jz(jump_arg_1, code_curr); // arg_zero_imag
    fstpi(code_curr, 0);
    fld_ptr_real(code_curr, in);
    // 0 if x > 0; pi if x < 0
    fldz(code_curr);
    fcomp(code_curr);
    fstpi(code_curr, 0);
    fstsw_ax(code_curr);
    sahf(code_curr);
    char * jump_arg_3 = code_curr;
    ja(code_curr, NULL); // arg_pi
    char * jump_arg_4 = code_curr;
    jb(code_curr, NULL); // arg_zero
    fldz(code_curr);
    stc(code_curr);
    char * jump_arg_5 = code_curr;
    jmp(code_curr, NULL); // arg_end
    ja(jump_arg_3, code_curr); // arg_pi
    fldpi(code_curr);
    char * jump_arg_6 = code_curr;
    jmp(code_curr, NULL); // arg_end
    jb(jump_arg_4, code_curr); // arg_zero
    fldz(code_curr);
    jmp(jump_arg_2, code_curr); // arg_end
    jmp(jump_arg_5, code_curr); // arg_end
    jmp(jump_arg_6, code_curr); // arg_end
    // if arg in (-pi,pi] transform it to [0,2pi]
    // we just calculate arg-2pi if arg > pi
    fldpi(code_curr);
    fcomp(code_curr);
    fstsw_ax(code_curr);
    sahf(code_curr);
    char * jump_arg_7 = code_curr;
    ja(code_curr, NULL); // arg_end_transform
    fldpi(code_curr);
    fld2(code_curr);
    fmul(code_curr);
    fsub(code_curr);
    ja(jump_arg_7, code_curr); // arg_end_transform
}

// Input:  in = X
// Output: out = arg(X)
template<typename T>
void complex_arg(char *& code_curr, const T * in, const T * out)
{
    complex_helper_abs(code_curr, in);
    complex_helper_arg(code_curr, in);
    //
    fstp_ptr_real(code_curr, out);
    fldz(code_curr);
    fstp_ptr_imag(code_curr, out);
}

// Input:  larg = X , rarg = Y
// Output: out = X ^ Y
template<typename T>
void complex_pow(char *& code_curr, const T * larg, const T * rarg, const T * out, const T * tmp)
{
    // pow(a, z) = r * cos(theta) + (r * sin(theta)) * I;
    // r = pow(Abs(a), Re(z)) * exp(-Im(z) * Arg(a));
    // theta = Re(z) * Arg(a) + Im(z) * log(Abs(a));
    //
    // Abs(a)
    complex_helper_abs(code_curr, larg);
    fldi(code_curr, 0);
    fstp_ptr_real(code_curr, tmp); // temp store for Abs(a)
    //
    // Arg(a)
    complex_helper_arg(code_curr, larg);
    fstp_ptr_imag(code_curr, tmp); // temp store for Arg(a)
    //
    // log(Abs(a))
    fld_ptr_real(code_curr, tmp);
    real_log(code_curr);
    //
    // theta = Re(z) * Arg(a) + Im(z) * log(Abs(a));
    fld_ptr_imag(code_curr, rarg);
    fmul(code_curr);
    fld_ptr_real(code_curr, rarg);
    fld_ptr_imag(code_curr, tmp);
    fmul(code_curr);
    fadd(code_curr);
    fstp_ptr_real(code_curr, tmp + 1); // temp store for theta
    //
    // pow(Abs(a), Re(z))
    fld_ptr_real(code_curr, rarg);
    fld_ptr_real(code_curr, tmp);
    real_pow(code_curr);
    fstp_ptr_imag(code_curr, tmp + 1); // temp store for pow
    //
    // exp(-Im(z) * Arg(a));
    fld_ptr_imag(code_curr, tmp);
    fld_ptr_imag(code_curr, rarg);
    fchs(code_curr);
    fmul(code_curr);
    real_exp(code_curr);
    //
    // r = pow(Abs(a), Re(z)) * exp(-Im(z) * Arg(a));
    fld_ptr_imag(code_curr, tmp + 1);
    fmul(code_curr);
    //
    // pow(a, z) = r * cos(theta) + (r * sin(theta)) * I;
    fld_ptr_real(code_curr, tmp + 1);
    fsincos(code_curr);
    fldi(code_curr, 2);
    fmul(code_curr);
    fstp_ptr_real(code_curr, out);
    fmul(code_curr);
    fstp_ptr_imag(code_curr, out);
}

// Input:  in = X
// Output: out = exp(X)
template<typename T>
void complex_exp(char *& code_curr, const T * in, const T * out)
{
    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
    //
    // exp(Re(z))
    fld_ptr_real(code_curr, in);
    real_exp(code_curr);
    // cos(Im(z)), sin(Im(z))
    fld_ptr_imag(code_curr, in);
    fsincos(code_curr);
    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
    fldi(code_curr, 2);
    fmul(code_curr);
    fstp_ptr_real(code_curr, out);
    fmul(code_curr);
    fstp_ptr_imag(code_curr, out);
}

// Input:  in = X
// Output: out = sin(X) if type == 's'
//               cos(X) if type == 'c'
template<typename T>
void complex_helper_sin_cos(char *& code_curr, const T * in, const T * out, const T * tmp, char type)
{
    // sin(x) = (exp(ix) - exp(-ix)) / 2i
    // cos(x) = (exp(ix) + exp(-ix)) / 2
    //
    // exp(ix)
    fld_ptr_imag(code_curr, in);
    fchs(code_curr);
    real_exp(code_curr);
    // cos(Im(z)), sin(Im(z))
    fld_ptr_real(code_curr, in);
    fsincos(code_curr);
    // -- begin magic
    fxch(code_curr);    // st0 - sin, st1 - cos, st2 - num
    fxch(code_curr, 2); // st0 - num, st1 - cos, st2 - sin
    fldi(code_curr, 2); // st0 - sin, st1 - num, st2 - cos, st3 - sin
    fldi(code_curr, 2); // st0 - cos, st1 - sin, st2 - num, st3 - cos, st4 - sin
    // -- end magic
    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
    fldi(code_curr, 2);
    fmul(code_curr);
    fstp_ptr_real(code_curr, tmp);
    fmul(code_curr);
    fstp_ptr_imag(code_curr, tmp);
    //
    // exp(-ix)
    fld_ptr_imag(code_curr, in);
    real_exp(code_curr);
    // cos(Im(z)), sin(Im(z))
    //fld_ptr_real(code_curr, px);
    //fchs(code_curr);
    //fsincos(code_curr);
    // -- begin magic
    fxch(code_curr, 2); // st0 - sin, st1 - cos, st2 - num
    fchs(code_curr);
    fxch(code_curr);    // st0 - cos, st1 - sin, st2 - num
    // -- end magic
    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
    fldi(code_curr, 2);
    fmul(code_curr);
    fstp_ptr_real(code_curr, tmp + 1);
    fmul(code_curr);
    fstp_ptr_imag(code_curr, tmp + 1);
    //
    if(type == 's')
    {
        //
        // exp(ix) - exp(-ix)
        fld_ptr_real(code_curr, tmp);
        fld_ptr_real(code_curr, tmp + 1);
        fsub(code_curr);
        fstp_ptr_real(code_curr, tmp);
        fld_ptr_imag(code_curr, tmp);
        fld_ptr_imag(code_curr, tmp + 1);
        fsub(code_curr);
        fstp_ptr_imag(code_curr, tmp);
        //
        // sin(x) = (exp(ix) - exp(-ix)) / 2i
        // (a+bi) / 2i = (b-ia) * 0.5 = 0.5b - 0.5a
        fld1(code_curr);
        fld2(code_curr);
        fdiv(code_curr);
        fldi(code_curr, 0);
        fld_ptr_imag(code_curr, tmp);
        fmul(code_curr);
        fstp_ptr_real(code_curr, out);
        fld_ptr_real(code_curr, tmp);
        fmul(code_curr);
        fchs(code_curr);
        fstp_ptr_imag(code_curr, out);
    }
    else
    {
        //
        // exp(ix) + exp(-ix)
        fld_ptr_real(code_curr, tmp);
        fld_ptr_real(code_curr, tmp + 1);
        fadd(code_curr);
        fstp_ptr_real(code_curr, tmp);
        fld_ptr_imag(code_curr, tmp);
        fld_ptr_imag(code_curr, tmp + 1);
        fadd(code_curr);
        fstp_ptr_imag(code_curr, tmp);
        //
        // cos(x) = (exp(ix) + exp(-ix)) * 0.5
        // (a+bi)*(c) = ac+bci
        fld1(code_curr);
        fld2(code_curr);
        fdiv(code_curr);
        fldi(code_curr, 0);
        fld_ptr_real(code_curr, tmp);
        fmul(code_curr);
        fstp_ptr_real(code_curr, out);
        fld_ptr_imag(code_curr, tmp);
        fmul(code_curr);
        fstp_ptr_imag(code_curr, out);
    }
}

// Input:  in = X
// Output: out = sin(X)
template<typename T>
void complex_sin(char *& code_curr, const T * in, const T * out, const T * tmp)
{
    complex_helper_sin_cos(code_curr, in, out, tmp, 's');
}

// Input:  in = X
// Output: out = cos(X)
template<typename T>
void complex_cos(char *& code_curr, const T * in, const T * out, const T * tmp)
{
    complex_helper_sin_cos(code_curr, in, out, tmp, 'c');
}

// Input:  in = X
// Output: out = tan(X)
template<typename T>
void complex_tan(char *& code_curr, const T * in, const T * out, const T * tmp)
{
    // tan(x) = (exp(ix) - exp(-ix)) / (i * (exp(ix) + exp(-ix)))
    //
    // exp(ix)
    fld_ptr_imag(code_curr, in);
    fchs(code_curr);
    real_exp(code_curr);
    // cos(Im(z)), sin(Im(z))
    fld_ptr_real(code_curr, in);
    fsincos(code_curr);
    // -- begin magic
    fxch(code_curr);    // st0 - sin, st1 - cos, st2 - num
    fxch(code_curr, 2); // st0 - num, st1 - cos, st2 - sin
    fldi(code_curr, 2); // st0 - sin, st1 - num, st2 - cos, st3 - sin
    fldi(code_curr, 2); // st0 - cos, st1 - sin, st2 - num, st3 - cos, st4 - sin
    // -- end magic
    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
    fldi(code_curr, 2);
    fmul(code_curr);
    fstp_ptr_real(code_curr, tmp);
    fmul(code_curr);
    fstp_ptr_imag(code_curr, tmp);
    //
    // exp(-ix)
    fld_ptr_imag(code_curr, in);
    real_exp(code_curr);
    // cos(Im(z)), sin(Im(z))
    //fld_ptr_real(code_curr, in);
    //fchs(code_curr);
    //fsincos(code_curr);
    // -- begin magic
    fxch(code_curr, 2); // st0 - sin, st1 - cos, st2 - num
    fchs(code_curr);
    fxch(code_curr);    // st0 - cos, st1 - sin, st2 - num
    // -- end magic
    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
    fldi(code_curr, 2);
    fmul(code_curr);
    fstp_ptr_real(code_curr, tmp + 1);
    fmul(code_curr);
    fstp_ptr_imag(code_curr, tmp + 1);
    //
    // exp(ix) - exp(-ix)
    fld_ptr_real(code_curr, tmp);
    fld_ptr_real(code_curr, tmp + 1);
    fsub(code_curr);
    fstp_ptr_real(code_curr, tmp + 2);
    fld_ptr_imag(code_curr, tmp);
    fld_ptr_imag(code_curr, tmp + 1);
    fsub(code_curr);
    fstp_ptr_imag(code_curr, tmp + 2);
    //
    // i * (exp(ix) + exp(-ix))
    fld_ptr_real(code_curr, tmp);
    fld_ptr_real(code_curr, tmp + 1);
    fadd(code_curr);
    fstp_ptr_imag(code_curr, tmp + 3);
    fld_ptr_imag(code_curr, tmp);
    fld_ptr_imag(code_curr, tmp + 1);
    fadd(code_curr);
    fchs(code_curr);
    fstp_ptr_real(code_curr, tmp + 3);
    //
    // tan(x) = (exp(ix) - exp(-ix)) / (i * (exp(ix) + exp(-ix)))
    complex_div(code_curr, tmp + 2, tmp + 3, out);
}

// Input:  in = X
// Output: st(0) = Im(log(X)) , st(1) = Re(log(X))
template<typename T>
void complex_helper_log(char *& code_curr, const T * in)
{
    // log(z) = ln(abs(z))+i*arg(z)
    //
    // abs(z)
    complex_helper_abs(code_curr, in);
    fldi(code_curr, 0);
    //
    // ln(abs(z))
    real_log(code_curr);
    fxch(code_curr);
    //
    // arg(z)
    complex_helper_arg(code_curr, in);
}

// Input:  in = X
// Output: out = log(X)
template<typename T>
void complex_log(char *& code_curr, const T * in, const T * out)
{
    complex_helper_log(code_curr, in);
    fstp_ptr_imag(code_curr, out);
    fstp_ptr_real(code_curr, out);
}

// Input:  in = X
// Output: out = log2(X)  if type == '2'
//               log10(X) if type == '0'
template<typename T>
void complex_helper_logN(char *& code_curr, const T * in, const T * out, char type)
{
    complex_helper_log(code_curr, in);
    if(type == '2')
    {
        fld1(code_curr);
        fldln2(code_curr);
    }
    else
    {
        fldl2e(code_curr);
        fldl2t(code_curr);
    }
    fdiv(code_curr);
    fxch(code_curr);
    fldi(code_curr, 1);
    // (a+bi)*c = (ac)+(bc)i
    fmul(code_curr);
    fstp_ptr_imag(code_curr, out);
    fmul(code_curr);
    fstp_ptr_real(code_curr, out);
}

// Input:  in = X
// Output: out = log2(X)
template<typename T>
void complex_log2(char *& code_curr, const T * in, const T * out)
{
    complex_helper_logN(code_curr, in, out, '2');
}

// Input:  in = X
// Output: out = log10(X)
template<typename T>
void complex_log10(char *& code_curr, const T * in, const T * out)
{
    complex_helper_logN(code_curr, in, out, '0');
}

// Input:  in = X
// Output: st(0) = Re(sqrt(X)) , Im(out) = Im(sqrt(X))
template<typename T>
void complex_helper_sqrt(char *& code_curr, const T * in, const T * out)
{
    // sqrt(z) = sqrt((|z| + Re(z))/2) + i * sign(Im(z)) * sqrt((|z| - Re(z))/2)
    //
    // sign(Im(z))
    // 1 if x > 0; -1 if x < 0; 0 if x == 0
    fld_ptr_imag(code_curr, in);
    fldz(code_curr);
    fcomp(code_curr);
    fstpi(code_curr, 0);
    fstsw_ax(code_curr);
    sahf(code_curr);
    char * jump_sign_1 = code_curr;
    ja(code_curr, NULL); // sign_m1
//    char * jump_sign_2 = curr;
//    jb(code_curr, NULL); // sign_1
//    //fldz(code_curr);
    fld1(code_curr); // TODO: WTF?
    char * jump_sign_3 = code_curr;
    jmp(code_curr, NULL); // sign_end
    ja(jump_sign_1, code_curr); // sign_m1
    fld1(code_curr);
    fchs(code_curr);
//    char * jump_sign_4 = curr;
//    jmp(code_curr, NULL); // sign_end
//    jb(jump_sign_2, code_curr); // sign_1
//    fld1(code_curr);
    jmp(jump_sign_3, code_curr); // sign_end
//    jmp(jump_sign_4, code_curr); // sign_end
    //
    // |z|
    fld_ptr_imag(code_curr, in);
    fldi(code_curr, 0);
    fmul(code_curr);
    fld_ptr_real(code_curr, in);
    fldi(code_curr, 0);
    fmul(code_curr);
    fadd(code_curr);
    fsqrt(code_curr);
    fxch(code_curr);
    fldi(code_curr, 1);
    //
    // sqrt((|z| - Re(z))/2)
    fld_ptr_real(code_curr, in);
    fsub(code_curr);
    fld2(code_curr);
    fdiv(code_curr);
    fsqrt(code_curr);
    //
    // sign(Im(z)) * sqrt((|z| - Re(z))/2)
    fmul(code_curr);
    fstp_ptr_imag(code_curr, out);
    //
    // sqrt((|z| + Re(z))/2)
    fld_ptr_real(code_curr, in);
    fadd(code_curr);
    fld2(code_curr);
    fdiv(code_curr);
    fsqrt(code_curr);
}

// Input:  in = X
// Output: out = sqrt(X)
template<typename T>
void complex_sqrt(char *& code_curr, const T * in, const T * out)
{
    complex_helper_sqrt(code_curr, in, out);
    fstp_ptr_real(code_curr, out);
}

// Input:  in = X
// Output: out = asin(X) if type == 's'
//               acos(X) if type == 'c'
template<typename T>
void complex_helper_asin_acos(char *& code_curr, const T * in, const T * out, const T * tmp, char type)
{
    // asin(z) = - i * log(i * z + sqrt((T)1 - z * z))
    // acos(z) = pi/2 - asin(z)
    //
    // 1 - z * z
    // 1-(a+bi)^2 = (1-aa+bb)-2abi
    fld_ptr_real(code_curr, in);
    fld_ptr_imag(code_curr, in);
    fldi(code_curr, 1);
    fldi(code_curr, 0);
    fmul(code_curr);
    fld1(code_curr);
    fsubr(code_curr);
    fldi(code_curr, 1);
    fldi(code_curr, 0);
    fmul(code_curr);
    fxch(code_curr);
    fadd(code_curr);
    fstp_ptr_real(code_curr, tmp);
    fmul(code_curr);
    fld2(code_curr);
    fmul(code_curr);
    fchs(code_curr);
    fstp_ptr_imag(code_curr, tmp);
    //
    // sqrt((T)1 - z * z)
    //complex_sqrt(code_curr, tmp, tmp);
    complex_helper_sqrt(code_curr, tmp, tmp);
    //
    // i * z + sqrt((T)1 - z * z)
    //fld_ptr_real(code_curr, tmp);
    fld_ptr_imag(code_curr, in);
    fsub(code_curr);
    fld_ptr_real(code_curr, in);
    fld_ptr_imag(code_curr, tmp);
    fadd(code_curr);
    fstp_ptr_imag(code_curr, tmp);
    fstp_ptr_real(code_curr, tmp);
    //
    // log(i * z + sqrt((T)1 - z * z))
    complex_helper_log(code_curr, tmp);
    //
    if(type == 's')
    {
        // asin(z) = - i * log(i * z + sqrt((T)1 - z * z))
        fstp_ptr_real(code_curr, out);
        fchs(code_curr);
        fstp_ptr_imag(code_curr, out);
    }
    else
    {
        // acos(z) = pi/2 - asin(z)
        fldpi(code_curr);
        fld2(code_curr);
        fdiv(code_curr);
        fsubr(code_curr);
        fstp_ptr_real(code_curr, out);
        fstp_ptr_imag(code_curr, out);
    }
}

// Input:  in = X
// Output: out = asin(X)
template<typename T>
void complex_asin(char *& code_curr, const T * in, const T * out, const T * tmp)
{
    complex_helper_asin_acos(code_curr, in, out, tmp, 's');
}

// Input:  in = X
// Output: out = acos(X)
template<typename T>
void complex_acos(char *& code_curr, const T * in, const T * out, const T * tmp)
{
    complex_helper_asin_acos(code_curr, in, out, tmp, 'c');
}

// Input:  in = X
// Output: out = atan(X)
template<typename T>
void complex_atan(char *& code_curr, const T * in, const T * out, const T * tmp)
{
    const T * l1 = tmp, * l2 = tmp + 1;
    // atan(z) = i/2 * (log(1-zi)-log(zi+1));
    //
    // 1-zi, 1+zi
    fld_ptr_imag(code_curr, in);
    fldi(code_curr, 0);
    fld1(code_curr);
    fadd(code_curr);
    fstp_ptr_real(code_curr, l1);
    fld1(code_curr);
    fsubr(code_curr);
    fstp_ptr_real(code_curr, l2);
    fld_ptr_real(code_curr, in);
    fldi(code_curr, 0);
    fchs(code_curr);
    fstp_ptr_imag(code_curr, l1);
    fstp_ptr_imag(code_curr, l2);
    //
    // log(1-zi), log(zi+1))
    complex_log(code_curr, l1, l1);
    complex_log(code_curr, l2, l2);
    //
    // atan(z) = (i * (log(1-zi)-log(zi+1))) * 0.5;
    // (a+bi)*(c) = (ac)+(bc)i
    fld2(code_curr);
    fld1(code_curr);
    fdivr(code_curr);
    fld_ptr_imag(code_curr, l2);
    fld_ptr_imag(code_curr, l1);
    fsub(code_curr);
    fldi(code_curr, 1);
    fmul(code_curr);
    fstp_ptr_real(code_curr, out);
    fld_ptr_real(code_curr, l1);
    fld_ptr_real(code_curr, l2);
    fsub(code_curr);
    fmul(code_curr);
    fstp_ptr_imag(code_curr, out);
}

// Input:  in = X
// Output: out = sinh(X) if type == 's'
//               cosh(X) if type == 'c'
//               tanh(X) if type == 't'
template<typename T>
void complex_helper_sinh_cosh_tanh(char *& code_curr, const T * in, const T * out, const T * tmp, char type)
{
    // sinh(z) = (exp(z) - exp(-z)) / 2
    // cosh(z) = (exp(z) + exp(-z)) / 2
    // tanh(z) = (exp(z) - exp(-z)) / (exp(z) + exp(-z))
    //
    // exp(x)
    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
    fld_ptr_real(code_curr, in);
    real_exp(code_curr);
    // cos(Im(z)), sin(Im(z))
    fld_ptr_imag(code_curr, in);
    fsincos(code_curr);
    // -- begin magic
    fxch(code_curr);    // st0 - sin, st1 - cos, st2 - num
    fxch(code_curr, 2); // st0 - num, st1 - cos, st2 - sin
    fldi(code_curr, 2); // st0 - sin, st1 - num, st2 - cos, st3 - sin
    fldi(code_curr, 2); // st0 - cos, st1 - sin, st2 - num, st3 - cos, st4 - sin
    // -- end magic
    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
    fldi(code_curr, 2);
    fmul(code_curr);
    fstp_ptr_real(code_curr, tmp);
    fmul(code_curr);
    fstp_ptr_imag(code_curr, tmp);
    //
    // exp(-x)
    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
    fld_ptr_real(code_curr, in);
    fchs(code_curr);
    real_exp(code_curr);
    // cos(Im(z)), sin(Im(z))
    //fld_ptr_real(code_curr, px);
    //fchs(code_curr);
    //fsincos(code_curr);
    // -- begin magic
    fxch(code_curr, 2); // st0 - sin, st1 - cos, st2 - num
    fchs(code_curr);
    fxch(code_curr);    // st0 - cos, st1 - sin, st2 - num
    // -- end magic
    // exp(z) = exp(Re(z)) * cos(Im(z)) + exp(Re(z)) * sin(Im(z)) * I;
    fldi(code_curr, 2);
    fmul(code_curr);
    fstp_ptr_real(code_curr, tmp + 1);
    fmul(code_curr);
    fstp_ptr_imag(code_curr, tmp + 1);
    //
    if(type == 's' || type == 'c')
    {
        if(type == 's')
        {
            // exp(x) - exp(-x)
            fld_ptr_real(code_curr, tmp);
            fld_ptr_real(code_curr, tmp + 1);
            fsub(code_curr);
            fstp_ptr_real(code_curr, tmp);
            fld_ptr_imag(code_curr, tmp);
            fld_ptr_imag(code_curr, tmp + 1);
            fsub(code_curr);
            fstp_ptr_imag(code_curr, tmp);
        }
        else
        {
            // exp(x) + exp(-x)
            fld_ptr_real(code_curr, tmp);
            fld_ptr_real(code_curr, tmp + 1);
            fadd(code_curr);
            fstp_ptr_real(code_curr, tmp);
            fld_ptr_imag(code_curr, tmp);
            fld_ptr_imag(code_curr, tmp + 1);
            fadd(code_curr);
            fstp_ptr_imag(code_curr, tmp);
        }
        //
        // sinh(z) = (exp(z) - exp(-z)) * 0.5
        // cosh(z) = (exp(z) + exp(-z)) * 0.5
        // (a+bi)*(c) = ac+bci
        fld1(code_curr);
        fld2(code_curr);
        fdiv(code_curr);
        fldi(code_curr, 0);
        fld_ptr_real(code_curr, tmp);
        fmul(code_curr);
        fstp_ptr_real(code_curr, out);
        fld_ptr_imag(code_curr, tmp);
        fmul(code_curr);
        fstp_ptr_imag(code_curr, out);
    }
    else
    {
        // exp(x) - exp(-x), exp(x) + exp(-x)
        fld_ptr_real(code_curr, tmp);
        fld_ptr_real(code_curr, tmp + 1);
        fldi(code_curr, 1);
        fldi(code_curr, 1);
        fsub(code_curr);
        fstp_ptr_real(code_curr, tmp);
        fadd(code_curr);
        fstp_ptr_real(code_curr, tmp + 1);
        fld_ptr_imag(code_curr, tmp);
        fld_ptr_imag(code_curr, tmp + 1);
        fldi(code_curr, 1);
        fldi(code_curr, 1);
        fsub(code_curr);
        fstp_ptr_imag(code_curr, tmp);
        fadd(code_curr);
        fstp_ptr_imag(code_curr, tmp + 1);
        //
        // tanh(z) = (exp(z) - exp(-z)) / (exp(z) + exp(-z))
        complex_div(code_curr, tmp, tmp + 1, out);
    }
}

// Input:  in = X
// Output: out = sinh(X)
template<typename T>
void complex_sinh(char *& code_curr, const T * in, const T * out, const T * tmp)
{
    complex_helper_sinh_cosh_tanh(code_curr, in, out, tmp, 's');
}

// Input:  in = X
// Output: out = cosh(X)
template<typename T>
void complex_cosh(char *& code_curr, const T * in, const T * out, const T * tmp)
{
    complex_helper_sinh_cosh_tanh(code_curr, in, out, tmp, 'c');
}

// Input:  in = X
// Output: out = tanh(X)
template<typename T>
void complex_tanh(char *& code_curr, const T * in, const T * out, const T * tmp)
{
    complex_helper_sinh_cosh_tanh(code_curr, in, out, tmp, 't');
}

// Input:  in = X
// Output: out = asinh(X) if type == 's'
//               acosh(X) if type == 'c'
template<typename T>
void complex_helper_asinh_acosh(char *& code_curr, const T * in, const T * out, const T * tmp, char type)
{
    // asinh(x) = log(x + sqrt(x * x + 1))
    // acosh(x) = log(x + sqrt(x * x - 1))
    //
    // x * x +- 1
    // (a+bi)^2 = (aa-bb+-1)+2abi
    fld_ptr_real(code_curr, in);
    fld_ptr_imag(code_curr, in);
    fldi(code_curr, 1);
    fldi(code_curr, 0);
    fmul(code_curr);
    fldi(code_curr, 1);
    fldi(code_curr, 0);
    fmul(code_curr);
    fsub(code_curr);
    fld1(code_curr);
    if(type == 's')
        fadd(code_curr);
    else
        fsub(code_curr);
    fstp_ptr_real(code_curr, tmp);
    fmul(code_curr);
    fld2(code_curr);
    fmul(code_curr);
    fstp_ptr_imag(code_curr, tmp);
    //
    // sqrt(x * x +- 1)
    //complex_sqrt(code_curr, tmp, tmp);
    complex_helper_sqrt(code_curr, tmp, tmp);
    //
    // x + sqrt(x * x +- 1)
    //fld_ptr_real(code_curr, tmp);
    fld_ptr_real(code_curr, in);
    fadd(code_curr);
    fstp_ptr_real(code_curr, tmp);
    fld_ptr_imag(code_curr, tmp);
    fld_ptr_imag(code_curr, in);
    fadd(code_curr);
    fstp_ptr_imag(code_curr, tmp);
    //
    // log(x + sqrt(x * x +- 1))
    complex_log(code_curr, tmp, out);
}

// Input:  in = X
// Output: out = asinh(X)
template<typename T>
void complex_asinh(char *& code_curr, const T * in, const T * out, const T * tmp)
{
    complex_helper_asinh_acosh(code_curr, in, out, tmp, 's');
}

// Input:  in = X
// Output: out = acosh(X)
template<typename T>
void complex_acosh(char *& code_curr, const T * in, const T * out, const T * tmp)
{
    complex_helper_asinh_acosh(code_curr, in, out, tmp, 'c');
}

// Input:  in = X
// Output: out = atanh(X)
template<typename T>
void complex_atanh(char *& code_curr, const T * in, const T * out, const T * tmp)
{
    // atanh(x) = 0.5 * log((1.0 + x) / (1.0 - x))
    //
    // 1.0 + x, 1.0 - x
    fld_ptr_real(code_curr, in);
    fldi(code_curr, 0);
    fld1(code_curr);
    fadd(code_curr);
    fxch(code_curr);
    fld1(code_curr);
    fsubr(code_curr);
    fstp_ptr_real(code_curr, tmp + 1);
    fstp_ptr_real(code_curr, tmp);
    fld_ptr_imag(code_curr, in);
    fldi(code_curr, 0);
    fchs(code_curr);
    fstp_ptr_imag(code_curr, tmp + 1);
    fstp_ptr_imag(code_curr, tmp);
    //
    // (1.0 + x) / (1.0 - x)
    complex_div(code_curr, tmp, tmp + 1, tmp);
    //
    // log((1.0 + x) / (1.0 - x))
    complex_helper_log(code_curr, tmp);
    //
    // atanh(x) = 0.5 * log((1.0 + x) / (1.0 - x))
    // (a+bi)*(c) = ac+bci
    fld1(code_curr);
    fld2(code_curr);
    fdiv(code_curr);
    fxch(code_curr, 2);
    fldi(code_curr, 2);
    fmul(code_curr);
    fstp_ptr_real(code_curr, out);
    fmul(code_curr);
    fstp_ptr_imag(code_curr, out);
}

} // namespace evaluator_internal_jit

#endif // EVALUATOR_COMPLEX_TEMPLATES_H

