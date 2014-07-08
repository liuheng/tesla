#include <tesla/base/CountdownLatch.h>

namespace tesla
{

namespace base
{

CountdownLatch::CountdownLatch(int count)
    : mutex_(),
      condition_(mutex_),
      count_(count)
{
}

void CountdownLatch::wait()
{
    MutexLockGuard lock(mutex_);
    while (count_ > 0)
    {
        condition_.wait();
    }
}

void CountdownLatch::countDown()
{
    MutexLockGuard lock(mutex_);
    --count_;
    if (count_ == 0)
    {
        condition_.notifyAll();
    }
}

int CountdownLatch::getCount() const
{
    MutexLockGuard lock(mutex_);
    return count_;
}

} // namespace base

} // namespace tesla
