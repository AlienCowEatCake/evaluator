#if !defined(EVALUATOR_TYPE_DETECTION_H)
#define EVALUATOR_TYPE_DETECTION_H

//#define EVALUATOR_NO_RTTI

#include <complex>
#include <string>
#include <limits>

#if !defined(EVALUATOR_NO_RTTI)
#include <typeinfo>
#endif

namespace evaluator_internal
{

// =============================================================================================

template<typename T>
inline bool is_floating()
{
    return false;
}

#define ADD_FLOATING_TYPE(TYPE) \
    template<> \
    inline bool is_floating<TYPE>() \
    { \
        return true; \
    }
    ADD_FLOATING_TYPE(float)
    ADD_FLOATING_TYPE(double)
    ADD_FLOATING_TYPE(long double)
#undef ADD_FLOATING_TYPE

template<typename T>
inline bool is_integer()
{
    return false;
}

#define ADD_INTEGER_TYPE(TYPE) \
    template<> \
    inline bool is_integer<TYPE>() \
    { \
        return true; \
    }
    ADD_INTEGER_TYPE(char)
    ADD_INTEGER_TYPE(signed char)
    ADD_INTEGER_TYPE(unsigned char)
    ADD_INTEGER_TYPE(signed short)
    ADD_INTEGER_TYPE(unsigned short)
    ADD_INTEGER_TYPE(signed int)
    ADD_INTEGER_TYPE(unsigned int)
    ADD_INTEGER_TYPE(signed long)
    ADD_INTEGER_TYPE(unsigned long)
    ADD_INTEGER_TYPE(signed long long)       /// @note C++11
    ADD_INTEGER_TYPE(unsigned long long)     /// @note C++11
#undef ADD_INTEGER_TYPE

namespace type_detection_p {

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

} // namespace type_detection_p

template<typename T>
inline bool is_complex()
{
    return type_detection_p::is_complex(static_cast<const T *>(NULL));
}

namespace type_detection_p {

template<typename T>
inline bool is_floating_complex(const std::complex<T> *)
{
    return is_floating<T>();
}

template<typename T>
inline bool is_floating_complex(const T *)
{
    return false;
}

} // namespace type_detection_p

template<typename T>
inline bool is_floating_complex()
{
    return type_detection_p::is_floating_complex(static_cast<const T *>(NULL));
}

// =============================================================================================

template<typename T>
inline bool is_float()
{
    return is_floating<T>() && sizeof(T) == 4;
}

template<typename T>
inline bool is_double()
{
    return is_floating<T>() && sizeof(T) == 8;
}

namespace type_detection_p {

template<typename T>
inline std::size_t sizeof_complex_part(const std::complex<T> *)
{
    return sizeof(T);
}

template<typename T>
inline std::size_t sizeof_complex_part(const T *)
{
    return std::numeric_limits<std::size_t>::max();
}

template<typename T>
inline std::size_t sizeof_complex_part()
{
    return sizeof_complex_part(static_cast<const T *>(NULL));
}

} // namespace type_detection_p

template<typename T>
inline bool is_complex_float()
{
    return is_floating_complex<T>() && type_detection_p::sizeof_complex_part<T>() == 4;
}

template<typename T>
inline bool is_complex_double()
{
    return is_floating_complex<T>() && type_detection_p::sizeof_complex_part<T>() == 8;
}

// =============================================================================================

template<typename T>
inline std::string get_type_name()
{
#if defined(EVALUATOR_NO_RTTI)
    return "unknown";
#else
    return typeid(T).name();
#endif
}

#define ADD_GET_TYPE_NAME(TYPE) \
    template<> \
    inline std::string get_type_name<TYPE>() \
    { \
        return std::string(#TYPE); \
    }
    ADD_GET_TYPE_NAME(signed char)
    ADD_GET_TYPE_NAME(unsigned char)
    ADD_GET_TYPE_NAME(char)
    ADD_GET_TYPE_NAME(signed short)
    ADD_GET_TYPE_NAME(unsigned short)
    ADD_GET_TYPE_NAME(signed int)
    ADD_GET_TYPE_NAME(unsigned int)
    ADD_GET_TYPE_NAME(signed long)
    ADD_GET_TYPE_NAME(unsigned long)
    ADD_GET_TYPE_NAME(signed long long)     /// @note C++11
    ADD_GET_TYPE_NAME(unsigned long long)   /// @note C++11
    ADD_GET_TYPE_NAME(float)
    ADD_GET_TYPE_NAME(double)
    ADD_GET_TYPE_NAME(long double)
    ADD_GET_TYPE_NAME(std::complex<float>)
    ADD_GET_TYPE_NAME(std::complex<double>)
    ADD_GET_TYPE_NAME(std::complex<long double>)
#undef ADD_GET_TYPE_NAME

// =============================================================================================

} // namespace evaluator_internal

#endif // EVALUATOR_TYPE_DETECTION_H

