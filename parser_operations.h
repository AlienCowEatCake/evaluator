#if !defined PARSER_OPERATIONS_H
#define PARSER_OPERATIONS_H

#include <cmath>
#include <complex>
#include <utility>
#include <map>
#include <string>
#include <typeinfo>
#include "parser_internal.h"

// Parser's functions, operators and constants
namespace parser_internal
{
    using namespace std;

    // =============================================================================================

    // All fultions (must NOT be inline).

    template<typename T> T pi_sin(const T & ar)
    {
        return sin(ar);
    }

    template<typename T> T pi_cos(const T & ar)
    {
        return cos(ar);
    }

    template<typename T> T pi_tan(const T & ar)
    {
        return tan(ar);
    }

    template<typename T> T pi_sinh(const T & ar)
    {
        return sinh(ar);
    }

    template<typename T> T pi_cosh(const T & ar)
    {
        return cosh(ar);
    }

    template<typename T> T pi_tanh(const T & ar)
    {
        return tanh(ar);
    }

    template<typename T> T pi_log(const T & ar)
    {
        return log(ar);
    }

    template<typename T> T pi_log10(const T & ar)
    {
        return log10(ar);
    }

    template<typename T> T pi_exp(const T & ar)
    {
        return exp(ar);
    }

    template<typename T> T pi_sqrt(const T & ar)
    {
        return sqrt(ar);
    }

    template<typename T> T pi_log2(const T & ar)
    {
        return log(ar) / log((T)2);
    }

    // http://functions.wolfram.com/ElementaryFunctions/ArcSinh/02/
    template<typename T> T pi_asinh(const T & ar)
    {
        return log(ar + sqrt(ar * ar + (T)1));
    }

    // http://functions.wolfram.com/ElementaryFunctions/ArcCosh/02/
    template<typename T> T pi_acosh(const T & ar)
    {
        return log(ar + sqrt(ar * ar - (T)1));
    }

    // http://functions.wolfram.com/ElementaryFunctions/ArcTanh/02/
    template<typename T> T pi_atanh(const T & ar)
    {
        return (T)0.5 * log(((T)1 + ar) / ((T)1 - ar));
    }

    template<typename T> complex<T> pi_abs(const complex<T> & ar)
    {
        return abs(ar);
    }

    // http://functions.wolfram.com/ElementaryFunctions/ArcSin/02/
    template<typename T> complex<T> pi_asin(const complex<T> & ar)
    {
        complex<T> I = complex<T>(0, 1);
        return - I * log(I * ar + sqrt((T)1 - ar * ar));
    }

    // http://functions.wolfram.com/ElementaryFunctions/ArcCos/02/
    template<typename T> complex<T> pi_acos(const complex<T> & ar)
    {
        return (T)3.14159265358979323846264338327950 / (T)2 - pi_asin(ar);
    }

    // http://functions.wolfram.com/ElementaryFunctions/ArcTan/02/
    template<typename T> complex<T> pi_atan(const complex<T> & ar)
    {
        complex<T> I = complex<T>(0, 1);
        return I / (T)2 * (log((T)1 - I * ar) - log(I * ar + (T)1));
    }

    template<typename T> complex<T> pi_imag(const complex<T> & ar)
    {
        return ar.imag();
    }

    template<typename T> complex<T> pi_real(const complex<T> & ar)
    {
        return ar.real();
    }

    template<typename T> complex<T> pi_conj(const complex<T> & ar)
    {
        return conj(ar);
    }

    template<typename T> complex<T> pi_arg(const complex<T> & ar)
    {
        return arg(ar);
    }

    template<typename T> T pi_abs(const T & ar)
    {
        return fabs(ar);
    }

    template<typename T> T pi_asin(const T & ar)
    {
        return asin(ar);
    }

    template<typename T> T pi_acos(const T & ar)
    {
        return acos(ar);
    }

    template<typename T> T pi_atan(const T & ar)
    {
        return atan(ar);
    }

    template<typename T> T pi_imag(const T & ar)
    {
        (void)ar;
        return (T)0;
    }

    template<typename T> T pi_real(const T & ar)
    {
        return ar;
    }

    template<typename T> T pi_conj(const T & ar)
    {
        return ar;
    }

    template<typename T> T pi_arg(const T & ar)
    {
        return arg(complex<T>(ar));
    }

    // =============================================================================================

    // Add function pointers into container.
    template<typename T>
    void init_functions(map<string, T(*)(const T &)> & funcs_map)
    {
        funcs_map["imag"]  = pi_imag;
        funcs_map["real"]  = pi_real;
        funcs_map["conj"]  = pi_conj;
        funcs_map["arg"]   = pi_arg;
        funcs_map["sin"]   = pi_sin;
        funcs_map["cos"]   = pi_cos;
        funcs_map["tan"]   = pi_tan;
        funcs_map["asin"]  = pi_asin;
        funcs_map["acos"]  = pi_acos;
        funcs_map["atan"]  = pi_atan;
        funcs_map["sinh"]  = pi_sinh;
        funcs_map["cosh"]  = pi_cosh;
        funcs_map["tanh"]  = pi_tanh;
        funcs_map["asinh"] = pi_asinh;
        funcs_map["acosh"] = pi_acosh;
        funcs_map["atanh"] = pi_atanh;
        funcs_map["log"]   = pi_log;
        funcs_map["log2"]  = pi_log2;
        funcs_map["log10"] = pi_log10;
        funcs_map["abs"]   = pi_abs;
        funcs_map["exp"]   = pi_exp;
        funcs_map["sqrt"]  = pi_sqrt;
    }

    // =============================================================================================

    // All operators (must NOT be inline).

    template<typename T> T pi_plus(const T & larg, const T & rarg)
    {
        return larg + rarg;
    }

    template<typename T> T pi_minus(const T & larg, const T & rarg)
    {
        return larg - rarg;
    }

    template<typename T> T pi_mult(const T & larg, const T & rarg)
    {
        return larg * rarg;
    }

    template<typename T> T pi_div(const T & larg, const T & rarg)
    {
        return larg / rarg;
    }

    template<typename T> T pi_pow(const T & larg, const T & rarg)
    {
        return pow(larg, rarg);
    }

    // =============================================================================================

    // Add operators pointers into container.
    template<typename T>
    void init_operators(map<char, pair<unsigned short int, T(*)(const T &, const T &)> > & opers_map)
    {
        typedef pair<unsigned short int, T(*)(const T &, const T &)> oper_type;
        opers_map['+'] = oper_type(1, pi_plus);
        opers_map['-'] = oper_type(1, pi_minus);
        opers_map['*'] = oper_type(2, pi_mult);
        opers_map['/'] = oper_type(2, pi_div);
        opers_map['^'] = oper_type(3, pi_pow);
    }

    // =============================================================================================

    // Init default constant values.
    template<typename T>
    void init_variables(map<string, var_container<T> > & vars_map)
    {
        // No more annoying warnings about this cast! >_<
        // vars_map["pi"].value() = static_cast<T>(3.14159265358979323846264338327950);
        // vars_map["e"].value()  = static_cast<T>(2.71828182845904523536028747135266);
        vars_map["pi"].value() = static_cast<T>(4) * pi_atan(static_cast<T>(1));
        vars_map["e"].value() = pi_exp(static_cast<T>(1));
        if(typeid(T) == typeid(complex<float>) ||
           typeid(T) == typeid(complex<double>) ||
           typeid(T) == typeid(complex<long double>))
        {
            vars_map["i"].value() = pi_sqrt(static_cast<T>(-1));
            vars_map["j"].value() = pi_sqrt(static_cast<T>(-1));
        }
    }

    // =============================================================================================
}

#endif // PARSER_OPERATIONS_H

