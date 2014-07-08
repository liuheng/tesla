#include <tesla/base/Thread.h>
#include <tesla/base/CurrentThread.h>
#include <tesla/base/Exception.h>
#include <tesla/base/Logger.h>

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/weak_ptr.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

#include <boost/static_assert.hpp>

namespace tesla
{

namespace base
{

using namespace tesla::base;
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
using namespace std;
#else // __GXX_EXPERIMENTAL_CXX0X__
using namespace boost;
#endif // __GXX_EXPERIMENTAL_CXX0X__

__thread int t_cachedTid;
__thread char t_tidString[32];
__thread const char* t_threadName;

const bool sameType = boost::is_same<int, pid_t>::value;
BOOST_STATIC_ASSERT(sameType);

pid_t gettid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void afterFork()
{
    t_cachedTid = 0;
    t_threadName = "main";
    CurrentThread::tid();
    // no need to call pthread_atfork(NULL, NULL, &afterFork);
}

class ThreadNameInitializer
{
public:
    ThreadNameInitializer()
    {
        t_threadName = "main";
        CurrentThread::tid();
        pthread_atfork(NULL, NULL, &afterFork);
    }
};

ThreadNameInitializer init;

void CurrentThread::cacheTid()
{
    if (t_cachedTid == 0)
    {
        t_cachedTid = gettid();
        int n = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
        assert(n == 6);
        (void) n;
    }
}

int CurrentThread::tid()
{
    if (__builtin_expect(t_cachedTid == 0, 0))
    {
        cacheTid();
    }
    return t_cachedTid;
}

const char* CurrentThread::tidString()
{
    return t_tidString;
}

const char* CurrentThread::name()
{
    return t_threadName;
}

bool CurrentThread::isMainThread()
{
    return tid() == ::getpid();
}

void CurrentThread::sleepUsec(int64_t usec)
{
    struct timespec ts = { 0, 0 };
    ts.tv_sec = static_cast<time_t>(usec / Timestamp::MICROSECONDS_PER_SECOND);
    ts.tv_nsec = static_cast<long>(usec % Timestamp::MICROSECONDS_PER_SECOND * 1000);
    ::nanosleep(&ts, NULL);
}

struct ThreadData
{
    Thread::ThreadFunc func_;
    string name_;
    weak_ptr<pid_t> wkTid_;

    ThreadData(const Thread::ThreadFunc& func,
               const string& name,
               const shared_ptr<pid_t>& t)
        : func_(func),
          name_(name),
          wkTid_(t)
    {}

    void runInThread()
    {
        pid_t t = CurrentThread::tid();

        shared_ptr<pid_t> ptid = wkTid_.lock();
        if (ptid)
        {
            *ptid = t;
            ptid.reset();
        }

        t_threadName = name_.empty() ? "TeslaThread" : name_.c_str();
        ::prctl(PR_SET_NAME, t_threadName);
        try
        {
            func_();
            t_threadName = "finished";
        }
        catch (const Exception& ex)
        {
            t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            fprintf(stderr, "stack trace: %s\n", ex.stackBacktrace());
            abort();
        }
        catch (const std::exception& ex)
        {
            t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        }
        catch (...)
        {
            t_threadName = "crashed";
            fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
            throw; // rethrow
        }
    }
};

void* startThread(void* obj)
{
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return NULL;
}

AtomicInt32 Thread::nCreated_;

Thread::Thread(const ThreadFunc& func, const string& n)
    : started_(false),
      joined_(false),
      pthreadId_(0),
      tid_(new pid_t(0)),
      func_(func),
      name_(n)
{
    setDefaultName();
}

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
Thread::Thread(ThreadFunc&& func, const string& n)
    : started_(false),
      joined_(false),
      pthreadId_(0),
      tid_(new pid_t(0)),
      func_(std::move(func)),
      name_(n)
{
    setDefaultName();
}
#endif // __GXX_EXPERIMENTAL_CXX0X__

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        pthread_detach(pthreadId_);
    }
}

void Thread::setDefaultName()
{
    int num = nCreated_.atomicIncrement();
    if (name_.empty())
    {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}

void Thread::start()
{
    assert(!started_);
    started_ = true;
    // FIXME: move(func_)
    ThreadData* data = new ThreadData(func_, name_, tid_);
    if (pthread_create(&pthreadId_, NULL, &startThread, data))
    {
        started_ = false;
        delete data; // or no delete?
        LOG_SYSFATAL << "Failed in pthread_create";
    }
}

int Thread::join()
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}

} // namespace base

} // namespace tesla

