#include <tesla/net/Buffer.h>
#include <tesla/net/SockOps.h>

#include <errno.h>
#include <sys/uio.h>

namespace tesla
{

namespace net
{

using namespace tesla::base;

const char Buffer::CRLF[] = "\r\n";
const size_t Buffer::CHEAP_PERPEND;
const size_t Buffer::INITIAL_SIZE;

ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    // saved an ioctl()/FIONREAD call to tell how much to read
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin()+writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    // when there is enough space in this buffer, don't read into extrabuf.
    // by doing this, we read 128k-1 bytes at most
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *savedErrno = errno;
    }
    else if (implicit_cast<size_t>(n) <= writable)
    {
        writerIndex_ += n;
    }
    else
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    // if (n == writable + sizeof extrabuf)
    // {
    //   goto line_30;
    // }
    return n;
}

} // namespace net

} // namespace tesla
