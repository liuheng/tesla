#include <tesla/base/Exception.h>

//#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>

namespace tesla
{

namespace base
{

Exception::Exception(const char* msg)
    : message_(msg)
{
    stackWalk();
}

Exception::Exception(const string& msg)
    : message_(msg)
{
    stackWalk();
}

Exception::~Exception() throw ()
{
}

const char* Exception::what() const throw()
{
    return message_.c_str();
}

const char* Exception::stackBacktrace() const throw()
{
    return stack_.c_str();
}

void Exception::stackWalk()
{
    const int len = 200;
    void* buffer[len];
    int nptrs = ::backtrace(buffer, len);
    char** strings = ::backtrace_symbols(buffer, nptrs);
    if (strings)
    {
        for (int i = 0; i < nptrs; ++i)
        {
            // TODO demangle funcion name with abi::__cxa_demangle
            stack_.append(strings[i]);
            stack_.push_back('\n');
        }
        free(strings);
    }
}

} // namespace base

} // namespace tesla
