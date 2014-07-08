/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/17/2014
 * Description
 *     - Provide WeakCallback<>, as a weak reference object
 *
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_WEAKCALLBACK_HPP
#define TESLA_BASE_WEAKCALLBACK_HPP

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <functional>
#include <memory>
#else
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#endif

namespace tesla
{

namespace base
{
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L

// FIXME: support std::shared_ptr as well, maybe using template template parameters

template<typename Class, typename... Args>
class WeakCallback
{
public:

    WeakCallback(const std::weak_ptr<Class>& object,
                 const std::function<void (Class*, Args...)>& function)
        : object_(object), function_(function)
    {
    }

    // Default dtor, copy ctor and assignment are okay

    void operator()(Args&&... args) const
    {
        std::shared_ptr<Class> ptr(object_.lock());
        if (ptr)
        {
            function_(ptr.get(), std::forward<Args>(args)...);
        }
        else
        {
            LOG_TRACE << "object expired";
        }
    }

private:

    std::weak_ptr<Class> object_;
    std::function<void (Class*, Args...)> function_;
}; // class WeakCallback<>

template<typename Class, typename... Args>
WeakCallback<Class, Args...> makeWeakCallback(const std::shared_ptr<Class>& object,
        void (Class::*function)(Args...))
{
    return WeakCallback<Class, Args...>(object, function);
}

template<typename Class, typename... Args>
WeakCallback<Class, Args...> makeWeakCallback(const std::shared_ptr<Class>& object,
        void (Class::*function)(Args...) const)
{
    return WeakCallback<Class, Args...>(object, function);
}
#else

// the C++98/03 version doesn't support arguments, use the no-argument version

template<typename Class>
class WeakCallback
{
public:

    WeakCallback(const boost::weak_ptr<Class>& object,
                 const boost::function<void (Class*)>& function)
        : object_(object), function_(function)
    {
    }

    // Default dtor, copy ctor and assignment are okay

    void operator()() const
    {
        boost::shared_ptr<Class> ptr(object_.lock());
        if (ptr)
        {
            function_(ptr.get());
        }
        // else
        // {
        //   LOG_TRACE << "expired";
        // }
    }

private:

    boost::weak_ptr<Class> object_;
    boost::function<void (Class*)> function_;
}; //class WeakCallback

template<typename Class>
WeakCallback<Class> makeWeakCallback(const boost::shared_ptr<Class>& object,
                                     void (Class::*function)())
{
    return WeakCallback<Class>(object, function);
}

template<typename Class>
WeakCallback<Class> makeWeakCallback(const boost::shared_ptr<Class>& object,
                                     void (Class::*function)() const)
{
    return WeakCallback<Class>(object, function);
}

#endif  // __GXX_EXPERIMENTAL_CXX0X__

} // namespace base

} // namespace tesla

#endif // TESLA_BASE_WEAKCALLBACK_HPP