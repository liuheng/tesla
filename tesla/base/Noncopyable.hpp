/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/17/2014
 * Description
 *     Noncopyable, copyed from boost::noncopyable.hpp
 *     I don't want to include boost if <tr1> exists
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_NONCOPYABLE_HPP
#define TESLA_BASE_NONCOPYABLE_HPP

namespace tesla
{

namespace base
{

class Noncopyable
{
protected:
    Noncopyable() {}
    ~Noncopyable() {}
private:  // emphasize the following members are private
    Noncopyable( const Noncopyable& );
    const Noncopyable& operator=( const Noncopyable& );
}; // class Noncopyable

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_NONCOPYABLE_HPP
