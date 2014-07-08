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

#ifndef TESLA_NET_INETADDRESS_H
#define TESLA_NET_INETADDRESS_H

#include <tesla/base/Copyable.hpp>
#include <tesla/base/StringPiece.hpp>

#include <netinet/in.h>

namespace tesla
{

namespace net
{

///
/// Wrapper of sockaddr_in.
///
/// This is an POD interface class.
class InetAddress
    : public tesla::base::Copyable
{
public:
    /// Constructs an endpoint with given port number.
    /// Mostly used in TcpServer listening.
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);

    /// Constructs an endpoint with given ip and port.
    /// @c ip should be "1.2.3.4"
    InetAddress(tesla::base::StringArg ip, uint16_t port);

    /// Constructs an endpoint with given struct @c sockaddr_in
    /// Mostly used when accepting new connections
    InetAddress(const struct sockaddr_in& addr)
        : addr_(addr)
    { }

    tesla::base::string toIp() const;
    tesla::base::string toIpPort() const;

    // default copy/assignment are Okay

    const struct sockaddr_in& getSockAddrInet() const
    {
        return addr_;
    }
    void setSockAddrInet(const struct sockaddr_in& addr)
    {
        addr_ = addr;
    }

    uint32_t ipNetEndian() const
    {
        return addr_.sin_addr.s_addr;
    }
    uint16_t portNetEndian() const
    {
        return addr_.sin_port;
    }

    // resolve hostname to IP address, not changing port or sin_family
    // return true on success.
    // thread safe
    static bool resolve(tesla::base::StringArg hostname, InetAddress* result);
    // static std::vector<InetAddress> resolveAll(const char* hostname, uint16_t port = 0);

private:
    struct sockaddr_in addr_;
};

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_INETADDRESS_H
