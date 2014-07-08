/*
 *
 * Author:
 *     Thomas Liu
 * Date:
 *     4/17/2014
 * Description:
 *     Abstration for atomic types, whose operations are all atomic.
 *     Specially, give AtomicInt32 and AtomicInt64 types.
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_ATOMIC_HPP
#define TESLA_BASE_ATOMIC_HPP

#include <stdint.h>

namespace tesla
{
namespace base
{

//Atomic Type Abstraction
template <typename T>
class AtomicType
{
public:
    //constructor, copy_constructor, operator=
    AtomicType()
        : value_(0)
    {}

    AtomicType(const AtomicType &other)
        : value_(other.atomicGet())
    {}

    AtomicType &operator=(const AtomicType &other)
    {
        atomicSet(other.atomicGet());
    }

    //the following 3 return the original value
    T atomicGet()
    {
        return __sync_val_compare_and_swap(&value_, 0, 0);
    }

    T atomicSet(const T &newValue)
    {
        return __sync_lock_test_and_set(&value_, newValue);
    }

    T atomicGetAndAdd(const T &x)
    {
        return __sync_fetch_and_add(&value_, x);
    }

    //the following 3 return the added value
    T atomicAddAndGet(const T &x)
    {
        return atomicGetAndAdd(x) + x;
    }

    T atomicIncrement()
    {
        return atomicAddAndGet(1);
    }

    T atomicDecrement()
    {
        return atomicAddAndGet(-1);
    }

private:
    volatile T value_;

}; // class AtomicType<>

typedef base::AtomicType<int32_t> AtomicInt32;
typedef base::AtomicType<int64_t> AtomicInt64;

} // namespace base

} // namespace tesla

#endif // TESLA_BASE_ATOMIC_HPP