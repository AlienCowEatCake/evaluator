#ifndef EVALUATOR_OPERATIONS_H
#define EVALUATOR_OPERATIONS_H

#include <cmath>
#include <complex>
#include <utility>
#include <map>
#include <string>
#include <typeinfo>
#include "evaluator_internal/var_container.h"

namespace evaluator_internal
{
    // =============================================================================================

    // All fultions (must NOT be inline).

    template<typename T> T eval_sin(const T & ar)
    {
        return sin(ar);
    }

    template<typename T> T eval_cos(const T & ar)
    {
        return cos(ar);
    }

    template<typename T> T eval_tan(const T & ar)
    {
        return tan(ar);
    }

    template<typename T> T eval_sinh(const T & ar)
    {
        return sinh(ar);
    }

    template<typename T> T eval_cosh(const T & ar)
    {
        return cosh(ar);
    }

    template<typename T> T eval_tanh(const T & ar)
    {
        return tanh(ar);
    }

    template<typename T> T eval_log(const T & ar)
    {
        return log(ar);
    }

    template<typename T> T eval_log10(const T & ar)
    {
        return log10(ar);
    }

    template<typename T> T eval_exp(const T & ar)
    {
        return exp(ar);
    }

    template<typename T> T eval_sqrt(const T & ar)
    {
        return sqrt(ar);
    }

    template<typename T> T eval_log2(const T & ar)
    {
        return log(ar) / log((T)2);
    }

    // http://functions.wolfram.com/ElementaryFunctions/ArcSinh/02/
    template<typename T> T eval_asinh(const T & ar)
    {
        return log(ar + sqrt(ar * ar + (T)1));
    }

    // http://functions.wolfram.com/ElementaryFunctions/ArcCosh/02/
    template<typename T> T eval_acosh(const T & ar)
    {
        return log(ar + sqrt(ar * ar - (T)1));
    }

    // http://functions.wolfram.com/ElementaryFunctions/ArcTanh/02/
    template<typename T> T eval_atanh(const T & ar)
    {
        return (T)0.5 * log(((T)1 + ar) / ((T)1 - ar));
    }

    template<typename T> std::complex<T> eval_abs(const std::complex<T> & ar)
    {
        return abs(ar);
    }

    // http://functions.wolfram.com/ElementaryFunctions/ArcSin/02/
    template<typename T> std::complex<T> eval_asin(const std::complex<T> & ar)
    {
        std::complex<T> I = std::complex<T>(0, 1);
        return - I * log(I * ar + sqrt((T)1 - ar * ar));
    }

    // http://functions.wolfram.com/ElementaryFunctions/ArcCos/02/
    template<typename T> std::complex<T> eval_acos(const std::complex<T> & ar)
    {
        return (T)3.14159265358979323846264338327950 / (T)2 - eval_asin(ar);
    }

    // http://functions.wolfram.com/ElementaryFunctions/ArcTan/02/
    template<typename T> std::complex<T> eval_atan(const std::complex<T> & ar)
    {
        std::complex<T> I = std::complex<T>(0, 1);
        return I / (T)2 * (log((T)1 - I * ar) - log(I * ar + (T)1));
    }

    template<typename T> std::complex<T> eval_imag(const std::complex<T> & ar)
    {
        return ar.imag();
    }

    template<typename T> std::complex<T> eval_real(const std::complex<T> & ar)
    {
        return ar.real();
    }

    template<typename T> std::complex<T> eval_conj(const std::complex<T> & ar)
    {
        return conj(ar);
    }

    template<typename T> std::complex<T> eval_arg(const std::complex<T> & ar)
    {
        return arg(ar);
    }

    template<typename T> T eval_abs(const T & ar)
    {
        return fabs(ar);
    }

    template<typename T> T eval_asin(const T & ar)
    {
        return asin(ar);
    }

    template<typename T> T eval_acos(const T & ar)
    {
        return acos(ar);
    }

    template<typename T> T eval_atan(const T & ar)
    {
        return atan(ar);
    }

    template<typename T> T eval_imag(const T & ar)
    {
        (void)ar;
        return (T)0;
    }

    template<typename T> T eval_real(const T & ar)
    {
        return ar;
    }

    template<typename T> T eval_conj(const T & ar)
    {
        return ar;
    }

    template<typename T> T eval_arg(const T & ar)
    {
        return arg(std::complex<T>(ar));
    }

    // =============================================================================================

    // Add function pointers into container.
    template<typename T>
    void init_functions(std::map<std::string, T(*)(const T &)> & funcs_map)
    {
        funcs_map["imag"]  = eval_imag;
        funcs_map["real"]  = eval_real;
        funcs_map["conj"]  = eval_conj;
        funcs_map["arg"]   = eval_arg;
        funcs_map["sin"]   = eval_sin;
        funcs_map["cos"]   = eval_cos;
        funcs_map["tan"]   = eval_tan;
        funcs_map["asin"]  = eval_asin;
        funcs_map["acos"]  = eval_acos;
        funcs_map["atan"]  = eval_atan;
        funcs_map["sinh"]  = eval_sinh;
        funcs_map["cosh"]  = eval_cosh;
        funcs_map["tanh"]  = eval_tanh;
        funcs_map["asinh"] = eval_asinh;
        funcs_map["acosh"] = eval_acosh;
        funcs_map["atanh"] = eval_atanh;
        funcs_map["log"]   = eval_log;
        funcs_map["log2"]  = eval_log2;
        funcs_map["log10"] = eval_log10;
        funcs_map["abs"]   = eval_abs;
        funcs_map["exp"]   = eval_exp;
        funcs_map["sqrt"]  = eval_sqrt;
    }

    // =============================================================================================

    // All operators (must NOT be inline).

    template<typename T> T eval_plus(const T & larg, const T & rarg)
    {
        return larg + rarg;
    }

    template<typename T> T eval_minus(const T & larg, const T & rarg)
    {
        return larg - rarg;
    }

    template<typename T> T eval_mult(const T & larg, const T & rarg)
    {
        return larg * rarg;
    }

    template<typename T> T eval_div(const T & larg, const T & rarg)
    {
        return larg / rarg;
    }

    template<typename T> T eval_pow(const T & larg, const T & rarg)
    {
        return pow(larg, rarg);
    }

    // =============================================================================================

    // Add operators pointers into container.
    template<typename T>
    void init_operators(std::map<char, std::pair<unsigned short int, T(*)(const T &, const T &)> > & opers_map)
    {
        typedef std::pair<unsigned short int, T(*)(const T &, const T &)> oper_data_type;
        opers_map['+'] = oper_data_type(1, eval_plus);
        opers_map['-'] = oper_data_type(1, eval_minus);
        opers_map['*'] = oper_data_type(2, eval_mult);
        opers_map['/'] = oper_data_type(2, eval_div);
        opers_map['^'] = oper_data_type(3, eval_pow);
    }

    // =============================================================================================

    // Init default constant values.
    template<typename T>
    void init_constants(std::map<std::string, T> & consts_map)
    {
        consts_map["pi"] = static_cast<T>(4) * eval_atan(static_cast<T>(1));
        consts_map["e"] = eval_exp(static_cast<T>(1));
        if(typeid(T) == typeid(std::complex<float>) ||
           typeid(T) == typeid(std::complex<double>) ||
           typeid(T) == typeid(std::complex<long double>))
        {
            consts_map["i"] = eval_sqrt(static_cast<T>(-1));
            consts_map["j"] = eval_sqrt(static_cast<T>(-1));
        }
    }

    // =============================================================================================
}

#endif // EVALUATOR_OPERATIONS_H

