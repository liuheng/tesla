/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/18/2014
 * Description
 *     base exception for tesla
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_EXCEPTION_H
#define TESLA_BASE_EXCEPTION_H

#include <tesla/base/Types.hpp>
#include <exception>

namespace tesla
{

namespace base
{

class Exception
    : public std::exception
{
public:
    explicit Exception(const char* what);
    explicit Exception(const string& what);
    virtual ~Exception() throw();
    virtual const char* what() const throw();
    const char* stackBacktrace() const throw();

private:
    void stackWalk();

    string message_;
    string stack_;
}; // class Exception

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_EXCEPTION_H
