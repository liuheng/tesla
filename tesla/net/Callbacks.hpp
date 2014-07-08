/*
 *
 * Author:
 *     Thomas Liu
 * Date:
 *     4/17/2014
 * Description:
 *
 *
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_NET_CALLBACKS_HPP
#define TESLA_NET_CALLBACKS_HPP

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <functional>
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

#include <tesla/base/Timestamp.h>

namespace tesla
{

namespace net
{

class Buffer;
class TcpConnection;

// Adapted from google-protobuf stubs/common.h
// see License in tesla/base/Types.hpp
template<typename To, typename From>
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
inline std::shared_ptr<To> down_pointer_cast(const ::boost::shared_ptr<From>& f)
#else // __GXX_EXPERIMENTAL_CXX0X__
inline boost::shared_ptr<To> down_pointer_cast(const ::boost::shared_ptr<From>& f)
#endif // __GXX_EXPERIMENTAL_CXX0X__
{
    if (false)
    {
        tesla::base::implicit_cast<From*, To*>(0);
    }

#ifndef NDEBUG
    assert(f == NULL || dynamic_cast<To*>(get_pointer(f)) != NULL);
#endif
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    return std::static_pointer_cast<To>(f);
#else // __GXX_EXPERIMENTAL_CXX0X__
    return boost::static_pointer_cast<To>(f);
#endif // __GXX_EXPERIMENTAL_CXX0X__
}

// All client visible callbacks go here.
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef boost::function<void()> TimerCallback;
typedef boost::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef boost::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef boost::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef boost::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

// the data has been read to (buf, len)
typedef boost::function<void (const TcpConnectionPtr&,
                              Buffer*,
                              tesla::base::Timestamp)> MessageCallback;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            tesla::base::Timestamp receiveTime);


} // namespace net

} // namespace tesla

#endif  // TESLA_NET_CALLBACKS_HPP
