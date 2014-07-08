/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/17/2014
 * Description
 *     class LogStream, to replace the std iostreams, more effective
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_LOGSTREAM_H
#define TESLA_BASE_LOGSTREAM_H

#include <tesla/base/StringPiece.hpp>
#include <tesla/base/Types.hpp>
#include <tesla/base/Noncopyable.hpp>

#include <string.h> // memcpy
#ifndef TESLA_USE_STD_STRING
#include <string>
#endif // TESLA_USE_STD_STRING

#include <assert.h>

namespace tesla
{

namespace base
{

const int SMALL_BUFFER = 4000;
const int LARGE_BUFFER = 4000*1000;

template<int SIZE>
class FixedBuffer
    : private Noncopyable
{
public:
    FixedBuffer()
        : cur_(data_)
    {
        setCookie(cookieStart);
    }

    ~FixedBuffer()
    {
        setCookie(cookieEnd);
    }

    void append(const char* /*restrict*/ buf, size_t len)
    {
        // FIXME: append partially
        if (implicit_cast<size_t>(available()) > len)
        {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    inline const char* data() const
    {
        return data_;
    }
    inline int length() const
    {
        return static_cast<int>(cur_ - data_);
    }

    // write to data_ directly
    inline char* current()
    {
        return cur_;
    }

    inline int available() const
    {
        return static_cast<int>(end() - cur_);
    }
    inline void add(size_t len)
    {
        cur_ += len;
    }
    inline void reset()
    {
        cur_ = data_;
    }
    inline void bzero()
    {
        ::bzero(data_, sizeof data_);
    }

    // for used by GDB
    const char* debugString();
    inline void setCookie(void (*cookie)())
    {
        cookie_ = cookie;
    }
    // for used by unit test
    inline string asString() const
    {
        return string(data_, length());
    }

private:
    inline const char* end() const
    {
        return data_ + sizeof data_;
    }

    // Must be outline function for cookies.
    static void cookieStart();
    static void cookieEnd();

    void (*cookie_)();
    char data_[SIZE];
    char* cur_;

}; // class FixedBuffer

/// a non-formatted simple but faster stream implementation
class LogStream
    : private Noncopyable
{
    typedef LogStream self;
public:
    typedef FixedBuffer<SMALL_BUFFER> Buffer;

    self& operator<<(bool v)
    {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }

    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);

    self& operator<<(const void*);

    self& operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    self& operator<<(double);
    // self& operator<<(long double);

    self& operator<<(char v)
    {
        buffer_.append(&v, 1);
        return *this;
    }

    // self& operator<<(signed char);
    // self& operator<<(unsigned char);

    self& operator<<(const char* str)
    {
        if (str)
        {
            buffer_.append(str, strlen(str));
        }
        else
        {
            buffer_.append("(null)", 6);
        }
        return *this;
    }

    self& operator<<(const unsigned char* str)
    {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    self& operator<<(const string& v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

#ifndef TESLA_USE_STD_STRING
    self& operator<<(const std::string& v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }
#endif // TESLA_USE_STD_STRING

    self& operator<<(const StringPiece& v)
    {
        buffer_.append(v.data(), v.size());
        return *this;
    }

    void append(const char* data, int len)
    {
        buffer_.append(data, len);
    }
    const Buffer& buffer() const
    {
        return buffer_;
    }
    void resetBuffer()
    {
        buffer_.reset();
    }

private:
    void staticCheck();

    template<typename T>
    void formatInteger(T);

    Buffer buffer_;

    static const int MAX_NUMERIC_SIZE = 32;
}; // class LogStream

class Fmt
    // : private Noncopyable
{
public:
    /// only accept lawable types T
    template<typename T>
    Fmt(const char* fmt, T val);

    const char* data() const
    {
        return buf_;
    }
    int length() const
    {
        return length_;
    }

private:
    char buf_[32];
    int length_;
}; // class Fmt

inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
    s.append(fmt.data(), fmt.length());
    return s;
}

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_LOGSTREAM_H
