#ifndef PARSER_H
#define PARSER_H

#include <map>
#include <set>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stack>
#include <deque>
#include <algorithm>
#include <cmath>
#include <utility>
#include <complex>
#include <typeinfo>
#include <limits>
#include <cctype>

#if defined _WIN32 || defined _WIN64
#if !defined NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#if (defined(_M_IX86 ) || defined(__X86__ ) || defined(__i386  ) || \
     defined(__IA32__) || defined(__I86__ ) || defined(__i386__) || \
     defined(__i486__) || defined(__i586__) || defined(__i686__))
#define PARSER_ARCH_X86
#endif

#if (defined(_M_X64  ) || defined(__x86_64) || defined(__x86_64__) || \
     defined(_M_AMD64) || defined(__amd64 ) || defined(__amd64__ ))
#define PARSER_ARCH_X64
#endif

//#define PARSER_ASM_DEBUG

#if defined(_MSC_VER) && _MSC_VER < 1800
namespace std
{
    // http://functions.wolfram.com/ElementaryFunctions/ArcCosh/02/
    template<typename T>
    T acosh(T x) { return log(x + sqrt(x * x - 1)); }
    // http://functions.wolfram.com/ElementaryFunctions/ArcSinh/02/
    template<typename T>
    T asinh(T x) { return log(x + sqrt(x * x + 1)); }
    // http://functions.wolfram.com/ElementaryFunctions/ArcTanh/02/
    template<typename T>
    T atanh(T x) { return  0.5 * log((1.0 + x) / (1.0 - x)); }
}
#endif

// =================================================================================================
// =================================================================================================
// =================================================================================================

// Internal parser's classes & functions
namespace parser_internal
{
    using namespace std;

    // =============================================================================================

    // Auto-allocatable container for variables.
    template<typename T> class var_container
    {
    protected:
        T * val;
    public:
        inline const T & value() const { return * val; }
        inline       T & value()       { return * val; }
        inline const T * pointer() const { return val; }
        inline       T * pointer()       { return val; }
        var_container(const T & new_val = T())     { val = new T(new_val); }
        var_container(const var_container & other) { val = new T(other.value()); }
        const var_container & operator = (const var_container & other) { val = new T; * val = other.value(); return * this; }
        ~var_container() { delete val; }
    };

    // =============================================================================================

    // All fultions (must NOT be inline).
    template<typename T> T pi_sin   (const T & arg) { return sin(arg);   }
    template<typename T> T pi_cos   (const T & arg) { return cos(arg);   }
    template<typename T> T pi_tan   (const T & arg) { return tan(arg);   }
    template<typename T> T pi_sinh  (const T & arg) { return sinh(arg);  }
    template<typename T> T pi_cosh  (const T & arg) { return cosh(arg);  }
    template<typename T> T pi_tanh  (const T & arg) { return tanh(arg);  }
    template<typename T> T pi_log   (const T & arg) { return log(arg);   }
    template<typename T> T pi_log10 (const T & arg) { return log10(arg); }
    template<typename T> T pi_exp   (const T & arg) { return exp(arg);   }
    template<typename T> T pi_sqrt  (const T & arg) { return sqrt(arg);  }
    template<typename T> T pi_log2  (const T & arg) { return log(arg) / log((T)2); }
    template<typename T> complex<T> pi_abs   (const complex<T> & arg) { return abs(arg);          }
    template<typename T> complex<T> pi_asin  (const complex<T> & arg) { return asin(arg.real());  }
    template<typename T> complex<T> pi_acos  (const complex<T> & arg) { return acos(arg.real());  }
    template<typename T> complex<T> pi_atan  (const complex<T> & arg) { return atan(arg.real());  }
    template<typename T> complex<T> pi_asinh (const complex<T> & arg) { return asinh(arg.real()); }
    template<typename T> complex<T> pi_acosh (const complex<T> & arg) { return acosh(arg.real()); }
    template<typename T> complex<T> pi_atanh (const complex<T> & arg) { return atanh(arg.real()); }
    template<typename T> complex<T> pi_imag  (const complex<T> & arg) { return arg.imag();        }
    template<typename T> complex<T> pi_real  (const complex<T> & arg) { return arg.real();        }
    template<typename T> complex<T> pi_conj  (const complex<T> & arg) { return conj(arg);         }
    template<typename T> T pi_abs   (const T & arg) { return fabs(arg);  }
    template<typename T> T pi_asin  (const T & arg) { return asin(arg);  }
    template<typename T> T pi_acos  (const T & arg) { return acos(arg);  }
    template<typename T> T pi_atan  (const T & arg) { return atan(arg);  }
    template<typename T> T pi_asinh (const T & arg) { return asinh(arg); }
    template<typename T> T pi_acosh (const T & arg) { return acosh(arg); }
    template<typename T> T pi_atanh (const T & arg) { return atanh(arg); }
    template<typename T> T pi_imag  (const T & arg) { return arg * (T)0; }
    template<typename T> T pi_real  (const T & arg) { return arg;        }
    template<typename T> T pi_conj  (const T & arg) { return arg;        }

    // =============================================================================================

    // Add function pointers into container.
    template<typename T>
    void init_functions(map<string, T(*)(const T &)> & funcs_map)
    {
        funcs_map["imag"]  = pi_imag;
        funcs_map["real"]  = pi_real;
        funcs_map["conj"]  = pi_conj;
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
    template<typename T> T pi_plus  (const T & larg, const T & rarg) { return larg + rarg; }
    template<typename T> T pi_minus (const T & larg, const T & rarg) { return larg - rarg; }
    template<typename T> T pi_mult  (const T & larg, const T & rarg) { return larg * rarg; }
    template<typename T> T pi_div   (const T & larg, const T & rarg) { return larg / rarg; }
    template<typename T> T pi_pow   (const T & larg, const T & rarg) { return pow(larg, rarg); }

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
    void init_constants(map<string, var_container<T> > & consts_map)
    {
        consts_map["pi"].value() = static_cast<T>(3.14159265358979323846264338327950);
        consts_map["e"].value()  = static_cast<T>(2.71828182845904523536028747135266);
        if(typeid(T) == typeid(complex<float>) ||
           typeid(T) == typeid(complex<double>) ||
           typeid(T) == typeid(complex<long double>))
        {
            consts_map["i"].value() = sqrt(static_cast<T>(-1.0));
            consts_map["j"].value() = sqrt(static_cast<T>(-1.0));
        }
    }

    // =============================================================================================

    // The universal parser object. It may be operator, function, variable or constant.
    template<typename T>
    class parser_object
    {
    protected:
        enum parser_object_type
        {
            PI_OBJ_OPERATOR, PI_OBJ_FUNCTION, PI_OBJ_VARIABLE, PI_OBJ_CONSTANT
        };
        parser_object_type type;
        string str_;
        T(* func)(const T &);
        T(* oper)(const T &, const T &);
        T value;
        const T * var_value;
        void init(parser_object_type n_type, const string & n_str, T(* n_func)(const T &),
                  T(* n_oper)(const T &, const T &), const T & n_value, const T * n_var_value)
        {
            type = n_type;
            str_ = n_str;
            value = n_value;
            var_value = n_var_value;
            func = n_func;
            oper = n_oper;
        }
    public:
        inline bool is_variable() const { return type == PI_OBJ_VARIABLE; }
        inline bool is_constant() const { return type == PI_OBJ_CONSTANT; }
        inline bool is_function() const { return type == PI_OBJ_FUNCTION; }
        inline bool is_operator() const { return type == PI_OBJ_OPERATOR; }
        inline const string & str() const { return str_; }
        inline const T * raw_value() const { return (type == PI_OBJ_VARIABLE ? var_value : (& value)); }
        inline T eval() const { return (type == PI_OBJ_VARIABLE ? (* var_value) : value); }
        inline T eval(const T & arg) const { return func(arg); }
        inline T eval(const T & larg, const T & rarg) const { return oper(larg, rarg); }
        parser_object(const string & new_str, const T * new_var_value)
        {
            init(PI_OBJ_VARIABLE, new_str, NULL, NULL, T(), new_var_value);
        }
        parser_object(const string & new_str, const T & new_value)
        {
            init(PI_OBJ_CONSTANT, new_str, NULL, NULL, new_value, NULL);
        }
        parser_object(const string & new_str, T(* new_func)(const T &))
        {
            init(PI_OBJ_FUNCTION, new_str, new_func, NULL, T(), NULL);
        }
        parser_object(const string & new_str, T(* new_oper)(const T &, const T &))
        {
            init(PI_OBJ_OPERATOR, new_str, NULL, new_oper, T(), NULL);
        }
    };

    // =============================================================================================

    template<typename T, const size_t max_size = 16>
    class simple_stack
    {
    public:
        inline const T & top() const
        {
            return data[top_place];
        }
        inline void pop()
        {
            top_place--;
        }
        inline size_t size() const
        {
            return top_place;
        }
        inline void push(const T & val)
        {
            data[++top_place] = val;
        }
        simple_stack()
        {
            top_place = 0;
        }
    protected:
        size_t top_place;
        T data[max_size];
    };
}

// =================================================================================================
// =================================================================================================
// =================================================================================================

// Internal parser's functions for opcode generation
namespace parser_opcodes_generator
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

    template<typename T>
    void fld_ptr(char *& code_curr, const T * ptr)
    {
#if defined PARSER_ARCH_X86
        // fld    [dq]word ptr ds:[ptr]
        if(typeid(T) == typeid(float))
            *(code_curr++) = '\xd9';
        else
            *(code_curr++) = '\xdd';
        *(code_curr++) = '\x05';
        const char * tmp_mem = reinterpret_cast<const char *>(ptr);
        memcpy(code_curr, & tmp_mem, sizeof(T*));
        code_curr += sizeof(T*);
        debug_asm_output("fld\t%cword ptr ds:[%xh]\n", (typeid(T) == typeid(float) ? 'd' : 'q'), (size_t)ptr);
#elif defined PARSER_ARCH_X64
        // mov    rdx, 0aaaaaaaaaaaaaaah
        *(code_curr++) = '\x48';
        *(code_curr++) = '\xba';
        const char * tmp_mem = reinterpret_cast<const char *>(ptr);
        memcpy(code_curr, & tmp_mem, sizeof(T*));
        code_curr += sizeof(T*);
        debug_asm_output("mov\trdx, %llxh\n", (size_t)ptr);
        // fld    [dq]word ptr [rdx]
        if(typeid(T) == typeid(float))
            *(code_curr++) = '\xd9';
        else
            *(code_curr++) = '\xdd';
        *(code_curr++) = '\x02';
        debug_asm_output("fld\t%cword ptr [rdx]\n", (typeid(T) == typeid(float) ? 'd' : 'q'));
#endif
    }

    template<typename T>
    void fstp_ptr(char *& code_curr, const T * ptr)
    {
#if defined PARSER_ARCH_X86
        // fstp    [dq]word ptr ds:[ptr]
        if(typeid(T) == typeid(float))
            *(code_curr++) = '\xd9';
        else
            *(code_curr++) = '\xdd';
        *(code_curr++) = '\x1d';
        const char * tmp_mem = reinterpret_cast<const char *>(ptr);
        memcpy(code_curr, & tmp_mem, sizeof(T*));
        code_curr += sizeof(T*);
        debug_asm_output("fstp\t%cword ptr ds:[%xh]\n", (typeid(T) == typeid(float) ? 'd' : 'q'), (size_t)ptr);
#elif defined PARSER_ARCH_X64
        // mov    rdx, 0aaaaaaaaaaaaaaah
        *(code_curr++) = '\x48';
        *(code_curr++) = '\xba';
        const char * tmp_mem = reinterpret_cast<const char *>(ptr);
        memcpy(code_curr, & tmp_mem, sizeof(T*));
        code_curr += sizeof(T*);
        debug_asm_output("mov\trdx, %llxh\n", (size_t)ptr);
        // fstp    [dq]word ptr [rdx]
        if(typeid(T) == typeid(float))
            *(code_curr++) = '\xd9';
        else
            *(code_curr++) = '\xdd';
        *(code_curr++) = '\x1a';
        debug_asm_output("fstp\t%cword ptr [rdx]\n", (typeid(T) == typeid(float) ? 'd' : 'q'));
#endif
    }

    inline void fadd(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xc1';
        debug_asm_output("fadd\n");
    }

    inline void fsub(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xe9';
        debug_asm_output("fsub\n");
    }

    inline void fmul(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xc9';
        debug_asm_output("fmul\n");
    }

    inline void fdiv(char *& code_curr)
    {
        *(code_curr++) = '\xde';
        *(code_curr++) = '\xf9';
        debug_asm_output("fdiv\n");
    }

    inline void fyl2x(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf1';
        debug_asm_output("fyl2x\n");
    }

    inline void f2xm1(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf0';
        debug_asm_output("f2xm1\n");
    }

    inline void fld1(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xe8';
        debug_asm_output("fld1\n");
    }

    inline void fxch(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xc9';
        debug_asm_output("fxch\n");
    }

    inline void fscale(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfd';
        debug_asm_output("fscale\n");
    }

    inline void fsin(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfe';
        debug_asm_output("fsin\n");
    }

    inline void fcos(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xff';
        debug_asm_output("fcos\n");
    }

    inline void fsqrt(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfa';
        debug_asm_output("fsqrt\n");
    }

    inline void fptan(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf2';
        debug_asm_output("fptan\n");
    }

    inline void fpatan(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xf3';
        debug_asm_output("fpatan\n");
    }

    inline void fldl2e(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xea';
        debug_asm_output("fldl2e\n");
    }

    inline void fldl2t(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xe9';
        debug_asm_output("fldl2t\n");
    }

    inline void fabs(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xe1';
        debug_asm_output("fabs\n");
    }

    inline void fldln2(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xed';
        debug_asm_output("fldln2\n");
    }

    inline void frndint(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xfc';
        debug_asm_output("frndint\n");
    }

    inline void fldi(char *& code_curr, int i)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xc0' + i;
        debug_asm_output("fldi\t%d\n", i);
    }

    inline void fchs(char *& code_curr)
    {
        *(code_curr++) = '\xd9';
        *(code_curr++) = '\xe0';
        debug_asm_output("fchs\n");
    }
}

// =================================================================================================
// =================================================================================================
// =================================================================================================

template<typename T>
class parser
{
protected:
    std::vector<std::string> expression;
    std::vector<parser_internal::parser_object<T> > expression_objects;
    std::map<std::string, T(*)(const T &)> functions;
    std::map<std::string, parser_internal::var_container<T> > constants;
    std::map<char, std::pair<unsigned short int, T(*)(const T &, const T &)> > operators;
    bool status;
    std::string error_string;
    bool is_compiled;
    char * volatile jit_code;
    size_t jit_code_size;
    volatile T jit_result;
    T * volatile jit_stack;
    size_t jit_stack_size;

    // =============================================================================================

    template<typename U>
    inline T incorrect_number(const std::complex<U> &) const
    {
        U num = std::numeric_limits<U>::max();
        return static_cast<T>(std::complex<U>(num, num));
    }

    template<typename U>
    inline T incorrect_number(const U &) const
    {
        return static_cast<T>(std::numeric_limits<U>::max());
    }

    template<typename U>
    inline bool is_incorrect(const std::complex<U> & val) const
    {
        static U num = std::numeric_limits<U>::max() * (static_cast<U>(1.0) - static_cast<U>(1e-3));
        return val.real() >= num && val.imag() >= num;
    }

    template<typename U>
    inline bool is_incorrect(const U & val) const
    {
        static U num = std::numeric_limits<U>::max() * (static_cast<U>(1.0) - static_cast<U>(1e-3));
        return val >= num;
    }

    // =============================================================================================

    void init()
    {
        using namespace std;
        using namespace parser_internal;
        status = false;
        is_compiled = false;
        jit_code = NULL;
        jit_code_size = 0;
        jit_stack = NULL;
        jit_stack_size = 0;
        init_functions(functions);
        init_operators(operators);
        init_constants(constants);
    }

    // =============================================================================================

    void convert_to_objects()
    {
        using namespace std;
        using namespace parser_internal;
        expression_objects.clear();
        expression_objects.reserve(expression.size());
        for(vector<string>::const_iterator it = expression.begin(); it != expression.end(); ++it)
        {
            typename map<string, var_container<T> >::const_iterator itc = constants.find(*it);
            if(itc != constants.end() && !is_incorrect(itc->second.value()))
            {
                expression_objects.push_back(parser_object<T>(*it, itc->second.value()));
            }
            else
            {
                typename map<char, pair<unsigned short int, T(*)(const T &, const T &)> >::const_iterator ito = operators.find((*it)[0]);
                if(ito != operators.end())
                {
                    expression_objects.push_back(parser_object<T>(*it, ito->second.second));
                }
                else
                {
                    typename map<string, T(*)(const T &)>::const_iterator itf = functions.find(*it);
                    if(itf != functions.end())
                    {
                        expression_objects.push_back(parser_object<T>(*it, itf->second));
                    }
                    else
                    {
                        constants[*it].value() = incorrect_number(T());
                        expression_objects.push_back(parser_object<T>(*it, constants[*it].pointer()));
                    }
                }
            }
        }
        expression.clear();
    }

    // =============================================================================================

    void copy_from_other_parser(const parser & other)
    {
        expression = other.expression;
        expression_objects = other.expression_objects;
        functions = other.functions;
        constants = other.constants;
        operators = other.operators;
        status = other.status;
        error_string = other.error_string;
        is_compiled = false;
        jit_code = NULL;
        jit_code_size = 0;
        jit_stack = NULL;
        jit_stack_size = 0;
    }

    // =============================================================================================

public:
    parser(const parser & other)
    {
        copy_from_other_parser(other);
    }

    const parser & operator = (const parser & other)
    {
        if(this != &other)
            copy_from_other_parser(other);
        return * this;
    }

    parser()
    {
        init();
    }

    parser(const std::string & str)
    {
        init();
        parse(str);
    }

    ~parser()
    {
        if(jit_code && jit_code_size)
        {
#if defined _WIN32 || defined _WIN64
            free(jit_code);
#else
            munmap(jit_code, jit_code_size);
#endif
        }
        if(jit_stack && jit_stack_size)
        {
            delete [] jit_stack;
        }
    }

    // =============================================================================================

    inline const std::string & get_error() const
    {
        return error_string;
    }

    // =============================================================================================

    inline void set_const(const std::string & name, const T & value)
    {
        constants[name].value() = value;
    }

    // =============================================================================================

    void reset_const()
    {
        using namespace std;
        using namespace parser_internal;
        constants.clear();
        init_constants(constants);
        if(is_parsed())
            for(typename vector<parser_object<T> >::iterator it = expression_objects.begin(); it != expression_objects.end(); ++it)
                constants[it->str()].value() = incorrect_number(T());
    }

    // =============================================================================================

    inline bool is_parsed() const
    {
        return status;
    }

    // =============================================================================================

    bool parse(const std::string & str)
    {
        using namespace std;
        using namespace parser_internal;

        expression.clear();
        expression_objects.clear();
        error_string = "";

        status = true;
        bool str_begin = true;
        bool unary_minus = false;
        stack<string> st;

        for(string::const_iterator it = str.begin(); it != str.end() && status;)
        {
            char sym = *it;
            if(sym >= '0' && sym <= '9')
            {
                string a;
                while(it != str.end() && sym >= '0' && sym <= '9')
                {
                    a.push_back(sym);
                    ++it;
                    if(it != str.end())
                    {
                        sym = *it;
                    }
                }
                if(it != str.end() && (sym == '.' || sym == ','))
                {
                    a.push_back('.');
                    ++it;
                    if(it != str.end())
                    {
                        sym = *it;
                    }
                    while(it != str.end() && sym >= '0' && sym <= '9')
                    {
                        a.push_back(sym);
                        ++it;
                        if(it != str.end())
                        {
                            sym = *it;
                        }
                    }
                }
                if(it != str.end() && (sym == 'e' || sym == 'E' || sym == 'd' || sym == 'D'))
                {
                    a.push_back('e');
                    ++it;
                    if(it != str.end())
                    {
                        sym = *it;
                    }
                    if(it != str.end() && (sym == '-' || sym == '+'))
                    {
                        a.push_back(sym);
                        ++it;
                        if(it != str.end())
                        {
                            sym = *it;
                        }
                    }
                    while(it != str.end() && sym >= '0' && sym <= '9')
                    {
                        a.push_back(sym);
                        ++it;
                        if(it != str.end())
                        {
                            sym = *it;
                        }
                    }
                }

                if(unary_minus)
                {
                    a = "-" + a;
                    unary_minus = false;
                }
                stringstream b;
                b << a;
                double c;
                b >> c;
                expression.push_back(a);
                constants[a].value() = static_cast<T>(c);
                str_begin = false;
            }
            else if(sym == '(')
            {
                st.push("(");
                str_begin = true;
                ++it;
            }
            else if(sym == ')')
            {
                while(!st.empty() && st.top() != "(")
                {
                    expression.push_back(st.top());
                    st.pop();
                }
                if(st.empty())
                {
                    status = false;
                    error_string = "Wrong brackets balance!";
                    break;
                }
                if(str_begin)
                {
                    status = false;
                    error_string = "Unexpected ')'!";
                    break;
                }
                st.pop();
                if(!st.empty() && functions.find(st.top()) != functions.end())
                {
                    expression.push_back(st.top());
                    st.pop();
                }
                str_begin = false;
                ++it;
            }
            else if(operators.find(sym) != operators.end())
            {
                if(sym == '-' && str_begin)
                {
                    string::const_iterator it2 = it + 1;
                    if(it2 == str.end())
                    {
                        status = false;
                        error_string = "Unexpected '-'!";
                        break;
                    }
                    if(*it2 >= '0' && *it2 <= '9')
                    {
                        unary_minus = true;
                    }
                    else
                    {
                        constants["-1.0"].value() = static_cast<T>(-1.0);
                        expression.push_back("-1.0");
                        st.push("*");
                    }
                }
                else
                {
                    char op;
                    if(!st.empty()) op = st.top().c_str()[0];
                    while(!st.empty() && operators.find(op) != operators.end() &&
                          operators[sym].first <= operators[op].first)
                    {
                        expression.push_back(st.top());
                        st.pop();
                        if(!st.empty()) op = st.top().c_str()[0];
                    }
                    string tmp;
                    tmp.push_back(sym);
                    st.push(tmp);
                }
                str_begin = false;
                ++it;
            }
            else if(sym != ' ' && sym != '\t' && sym != '\0' && sym != '\r' && sym != '\n')
            {
                string funcname;
                while(it != str.end() && *it != '(' && *it != ')' &&
                      *it != ' ' && *it != '\t' && *it != '\0' && *it != '\r' && *it != '\n' &&
                      operators.find(*it) == operators.end())
                {
                    funcname.push_back(*it);
                    ++it;
                }
                transform(funcname.begin(), funcname.end(), funcname.begin(), ::tolower);
                if(functions.find(funcname) != functions.end())
                {
                    st.push(funcname);
                }
                else if(it == str.end() || *it != '(')
                {
                    expression.push_back(funcname);
                }
                else
                {
                    status = false;
                    error_string = "Wrong function!";
                    break;
                }
                str_begin = false;
            }
            else
            {
                ++it;
            }
        }

        while(status && !st.empty())
        {
            if(operators.find(st.top().c_str()[0]) == operators.end())
            {
                status = false;
                error_string = "Wrong expression!";
                break;
            }
            expression.push_back(st.top());
            st.pop();
        }

        if(status) convert_to_objects();
        return status;
    }

    // =============================================================================================

    bool simplify()
    {
        using namespace std;
        using namespace parser_internal;

        if(!is_parsed())
        {
            error_string = "Not parsed!";
            return false;
        }

        bool was_changed;
        do
        {
            deque<parser_object<T> > dq;
            was_changed = false;

            for(typename vector<parser_object<T> >::iterator it = expression_objects.begin(); it != expression_objects.end(); ++it)
            {
                if(it->is_operator())
                {
                    parser_object<T> arg2 = dq.back();
                    if(arg2.is_constant())
                    {
                        dq.pop_back();
                        parser_object<T> arg1 = dq.back();
                        if(arg1.is_constant())
                        {
                            dq.pop_back();
                            T varg1 = arg1.eval();
                            T varg2 = arg2.eval();
                            T val = it->eval(varg1, varg2);
                            stringstream sst;
                            sst.precision(17);
                            sst.setf(ios::scientific);
                            sst << val;
                            string sst_st = sst.str();
                            dq.push_back(parser_object<T>(sst_st, val));
                        }
                        else
                        {
                            dq.push_back(arg2);
                            dq.push_back(*it);
                        }
                    }
                    else
                    {
                        dq.push_back(*it);
                    }
                }
                else if(it->is_function())
                {
                    parser_object<T> arg = dq.back();
                    if(arg.is_constant())
                    {
                        dq.pop_back();
                        T varg = arg.eval();
                        T val = it->eval(varg);
                        stringstream sst;
                        sst.precision(17);
                        sst.setf(ios::scientific);
                        sst << val;
                        string sst_st = sst.str();
                        dq.push_back(parser_object<T>(sst_st, val));
                    }
                    else
                    {
                        dq.push_back(*it);
                    }
                }
                else
                {
                    dq.push_back(*it);
                }
            }

            if(expression_objects.size() > dq.size())
            {
                expression_objects.clear();
                expression_objects.reserve(dq.size());
                while(!dq.empty())
                {
                    expression_objects.push_back(dq.front());
                    dq.pop_front();
                }
                was_changed = true;
            }
            else
            {
                dq.clear();
            }

            for(typename vector<parser_object<T> >::iterator it = expression_objects.begin(); it != expression_objects.end(); ++it)
            {
                if(it->is_operator())
                {
                    parser_object<T> arg2 = dq.back();
                    dq.pop_back();
                    parser_object<T> arg1 = dq.back();
                    // Such things as a*0 or 0*a
                    if(it->str() == "*" && ((arg2.is_constant() && arg2.eval() == static_cast<T>(0.0)) ||
                                            (arg1.is_constant() && arg1.eval() == static_cast<T>(0.0) &&
                                             !arg2.is_operator() && !arg2.is_function())))
                    {
                        dq.pop_back();
                        if(arg2.is_constant() && arg2.eval() == static_cast<T>(0.0))
                            dq.push_back(arg2);
                        else
                            dq.push_back(arg1);
                    }
                    // Such things as a*1 or 1*a
                    else if(it->str() == "*" && ((arg2.is_constant() && arg2.eval() == static_cast<T>(1.0)) ||
                                                 (arg1.is_constant() && arg1.eval() == static_cast<T>(1.0) &&
                                                  !arg2.is_operator() && !arg2.is_function())))
                    {
                        dq.pop_back();
                        if(arg2.is_constant() && arg2.eval() == static_cast<T>(1.0))
                            dq.push_back(arg1);
                        else
                            dq.push_back(arg2);
                    }
                    // Such things as a+0 or 0+a
                    else if(it->str() == "+" && ((arg2.is_constant() && arg2.eval() == static_cast<T>(0.0)) ||
                                                 (arg1.is_constant() && arg1.eval() == static_cast<T>(0.0) &&
                                                  !arg2.is_operator() && !arg2.is_function())))
                    {
                        dq.pop_back();
                        if(arg2.is_constant() && arg2.eval() == static_cast<T>(0.0))
                            dq.push_back(arg1);
                        else
                            dq.push_back(arg2);
                    }
                    // Such things as a-0
                    else if(it->str() == "-" && arg2.is_constant() && arg2.eval() == static_cast<T>(0.0))
                    {
                        dq.pop_back();
                        dq.push_back(arg1);
                    }
                    // Nothing...
                    else
                    {
                        dq.push_back(arg2);
                        dq.push_back(*it);
                    }
                }
                else
                {
                    dq.push_back(*it);
                }
            }

            if(expression_objects.size() > dq.size())
            {
                expression_objects.clear();
                expression_objects.reserve(dq.size());
                while(!dq.empty())
                {
                    expression_objects.push_back(dq.front());
                    dq.pop_front();
                }
                was_changed = true;
            }
            else
            {
                dq.clear();
            }
        }
        while(was_changed);
        return true;
    }

    // =============================================================================================

    bool calculate(T & result)
    {
        using namespace std;
        using namespace parser_internal;

        if(!is_parsed())
        {
            error_string = "Not parsed!";
            return false;
        }

        if(is_compiled)
        {
            typedef void(* jit_f_type)();
            jit_f_type func = reinterpret_cast<jit_f_type>(jit_code);
            func();
            result = jit_result;
            return true;
        }

        //stack<T> st;
        simple_stack<T> st;

        for(typename vector<parser_object<T> >::const_iterator it = expression_objects.begin(); it != expression_objects.end(); ++it)
        {
            if(it->is_constant())
            {
                st.push(it->eval());
            }
            else if(it->is_variable())
            {
                T val = it->eval();
                if(!is_incorrect(val))
                {
                    st.push(val);
                }
                else
                {
                    stringstream sst;
                    sst << "Constant `" << it->str() << "` must be defined!";
                    error_string = sst.str();
                    return false;
                }
            }
            else if(it->is_operator())
            {
                T arg2 = st.top();
                st.pop();
                T arg1 = st.top();
                st.pop();
                T val = it->eval(arg1, arg2);
                st.push(val);
            }
            else if(it->is_function())
            {
                T arg = st.top();
                st.pop();
                T val = it->eval(arg);
                st.push(val);
            }
        }

        if(st.size() != 1)
        {
            stringstream sst;
            sst << "Stack size equal " << st.size();
            error_string = sst.str();
            return false;
        }
        result = st.top();
        return true;
    }

    // =============================================================================================

public:

    bool compile()
    {
        using namespace std;
        using namespace parser_internal;
        using namespace parser_opcodes_generator;

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
            jit_code = mmap(NULL, jit_code_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
        }
        memset(jit_code, 0, jit_code_size);

        if(!jit_stack || !jit_stack_size)
        {
            jit_stack_size = 128 * 1024; // 128 KiB
            jit_stack = new T [jit_stack_size];
        }

        char * curr = jit_code;
        T * jit_stack_curr = jit_stack;

#if defined PARSER_ARCH_X86 || defined PARSER_ARCH_X64
        // http://www.intel-assembler.it/portale/5/The-8087-Instruction-Set/A-one-line-description-of-x87-instructions.asp

        if((typeid(T) == typeid(float) && sizeof(float) == 4) || (typeid(T) == typeid(double) && sizeof(double) == 8))
        {
            char * last_push_pos = NULL;
            T * last_push_val = NULL;
            for(typename vector<parser_object<T> >::const_iterator it = expression_objects.begin(); it != expression_objects.end(); ++it)
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
                        fxch(curr);
                        fsub(curr);
                        fsqrt(curr);
                        fpatan(curr);
                    }
                    else if(fu == "acos")
                    {
                        // arccos(a) = 2 * arctg(sqrt(1 - a) / sqrt(1 + a))
                        fldi(curr, 0);
                        fld1(curr);
                        fxch(curr);
                        fsub(curr);
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
                        fxch(curr);
                        fsub(curr);
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
                    else
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
            if(last_push_val == jit_stack_curr)
                curr = last_push_pos;
            else
                fld_ptr(curr, jit_stack_curr);

            fstp_ptr(curr, (T*)(& jit_result));
        }
        else if((typeid(T) == typeid(complex<float>) && sizeof(float) == 4) || (typeid(T) == typeid(complex<double>) && sizeof(double) == 8))
        {

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

        is_compiled = true;
        return true;
    }

    // =============================================================================================

    void debug_print() const
    {
        using namespace std;
        using namespace parser_internal;
        if(expression.size() > expression_objects.size())
            for(vector<string>::const_iterator it = expression.begin(); it != expression.end(); ++it)
                cout << * it << ' ';
        else
            for(typename vector<parser_object<T> >::const_iterator it = expression_objects.begin(); it != expression_objects.end(); ++it)
            {
                cout << it->str();
                if(it->is_variable())
                    cout << "->" << it->eval() << ' ';
                else
                    cout << ' ';
            }
        cout << endl;
    }
};

#endif // PARSER_H
