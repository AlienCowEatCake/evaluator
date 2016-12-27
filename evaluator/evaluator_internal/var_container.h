#if !defined(EVALUATOR_VAR_CONTAINER_H)
#define EVALUATOR_VAR_CONTAINER_H

namespace evaluator_internal
{

// Auto-allocatable container for variables.
template<typename T> class var_container
{
private:

    T * m_value;

public:

    inline const T & value() const
    {
        return * m_value;
    }

    inline T & value()
    {
        return * m_value;
    }

    inline const T * pointer() const
    {
        return m_value;
    }

    inline T * pointer()
    {
        return m_value;
    }

    var_container(const T & new_value = T())
    {
        m_value = new T(new_value);
    }

    var_container(const var_container & other)
    {
        m_value = new T(other.value());
    }

    const var_container & operator = (const var_container & other)
    {
        if(this != & other)
        {
            m_value = new T;
            * m_value = other.value();
        }
        return * this;
    }

    ~var_container()
    {
        delete m_value;
    }
};

} // namespace evaluator_internal

#endif // EVALUATOR_VAR_CONTAINER_H

