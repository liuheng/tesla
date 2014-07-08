#ifndef TESLA_BASE_LOGFILE_H
#define TESLA_BASE_LOGFILE_H

#include <tesla/base/Mutex.hpp>
#include <tesla/base/Types.hpp>
#include <tesla/base/Noncopyable.hpp>

#include <boost/scoped_ptr.hpp>

namespace tesla
{

namespace base
{

class FileAppender;

class FileLogger
    : private Noncopyable
{
public:
    FileLogger(const string& basename,
               size_t rollSize,
               bool threadSafe = true,
               int flushInterval = 3,
               int checkEveryN = 1024);
    ~FileLogger();

    void append(const char* logline, int len);
    void flush();
    bool rollFile();

private:
    void append_unlocked(const char* logline, int len);

    static string getLogFileName(const string& basename, time_t* now);

    const string basename_;
    const size_t rollSize_;
    const int flushInterval_;
    const int checkEveryN_;

    int count_;

    boost::scoped_ptr<MutexLock> mutex_;
    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;
    boost::scoped_ptr<FileAppender> file_;

    const static int ROLL_PER_SECONDS_ = 60*60*24;
}; // class FileLogger

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_LOGFILE_H
