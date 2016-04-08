#ifndef EVALUATOR_TYPE_DETECTION_H
#define EVALUATOR_TYPE_DETECTION_H

//#define EVALUATOR_NO_RTTI

#include <complex>
#include <string>

#if !defined(EVALUATOR_NO_RTTI)
#include <typeinfo>
#endif

namespace evaluator_internal
{

// =============================================================================================

inline bool is_floating(const float *)
{
    return true;
}

inline bool is_floating(const double *)
{
    return true;
}

inline bool is_floating(const long double *)
{
    return true;
}

template<typename T>
inline bool is_floating(const T *)
{
    return false;
}

template<typename T>
inline bool is_complex(const std::complex<T> *)
{
    return true;
}

template<typename T>
inline bool is_complex(const T *)
{
    return false;
}

template<typename T>
inline bool is_floating_complex(const std::complex<T> *)
{
    return is_floating(static_cast<T*>(NULL));
}

template<typename T>
inline bool is_floating_complex(const T *)
{
    return false;
}

// =============================================================================================

template<typename T>
inline bool is_float(const T *)
{
    return is_floating(static_cast<T*>(NULL)) && sizeof(T) == 4;
}

template<typename T>
inline bool is_double(const T *)
{
    return is_floating(static_cast<T*>(NULL)) && sizeof(T) == 8;
}

template<typename T>
inline bool is_complex_float(const std::complex<T> *)
{
    return sizeof(T) == 4;
}

template<typename T>
inline bool is_complex_float(const T *)
{
    return false;
}

template<typename T>
inline bool is_complex_double(const std::complex<T> *)
{
    return sizeof(T) == 8;
}

template<typename T>
inline bool is_complex_double(const T *)
{
    return false;
}

// =============================================================================================

#if defined(EVALUATOR_NO_RTTI)

inline std::string get_type_name(const signed char *)
{
    return "signed char";
}

inline std::string get_type_name(const unsigned char *)
{
    return "unsigned char";
}

inline std::string get_type_name(const char *)
{
    return "char";
}

inline std::string get_type_name(const signed short *)
{
    return "signed short";
}

inline std::string get_type_name(const unsigned short *)
{
    return "unsigned short";
}

inline std::string get_type_name(const signed int *)
{
    return "signed int";
}

inline std::string get_type_name(const unsigned int *)
{
    return "unsigned int";
}

inline std::string get_type_name(const signed long *)
{
    return "signed long";
}

inline std::string get_type_name(const unsigned long *)
{
    return "unsigned long";
}

inline std::string get_type_name(const signed long long *)
{
    return "signed long long";
}

inline std::string get_type_name(const unsigned long long *)
{
    return "unsigned long long";
}

inline std::string get_type_name(const float *)
{
    return "float";
}

inline std::string get_type_name(const double *)
{
    return "double";
}

inline std::string get_type_name(const long double *)
{
    return "long double";
}

inline std::string get_type_name(const std::complex<float> *)
{
    return "complex<float>";
}

inline std::string get_type_name(const std::complex<double> *)
{
    return "complex<double>";
}

inline std::string get_type_name(const std::complex<long double> *)
{
    return "complex<long double>";
}

template<typename T>
inline std::string get_type_name(const T *)
{
    return "unknown";
}

#else

template<typename T>
inline std::string get_type_name(const T *)
{
    return typeid(T).name();
}

#endif

// =============================================================================================

} // namespace evaluator_internal

#endif // EVALUATOR_TYPE_DETECTION_H

