#ifndef VAR_CONTAINER_H
#define VAR_CONTAINER_H

namespace evaluator_internal
{

// Auto-allocatable container for variables.
template<typename T> class var_container
{
protected:

    T * val;

public:

    inline const T & value() const
    {
        return * val;
    }

    inline T & value()
    {
        return * val;
    }

    inline const T * pointer() const
    {
        return val;
    }

    inline T * pointer()
    {
        return val;
    }

    var_container(const T & new_val = T())
    {
        val = new T(new_val);
    }

    var_container(const var_container & other)
    {
        val = new T(other.value());
    }

    const var_container & operator = (const var_container & other)
    {
        if(this != & other)
        {
            val = new T;
            * val = other.value();
        }
        return * this;
    }

    ~var_container()
    {
        delete val;
    }
};

}

#endif // VAR_CONTAINER_H

