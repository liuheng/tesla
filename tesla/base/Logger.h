/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/17/2014
 * Description
 *     Implementation for a simple, synchronic, stream-based logger
 *     only support loglevel, appenders, layouts, filters are not implemented.
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_LOGGING_H
#define TESLA_BASE_LOGGING_H

#include <tesla/base/LogStream.h>
#include <tesla/base/Timestamp.h>

namespace tesla
{

namespace base
{

class Time;

class Logger
{
public:
    // LogLevel
    enum LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_OF_LOG_LEVELS,
    };

    // internal class, compile time calculation of basename of source file
    class SourceFile
    {
    public:
        template<int N>
        inline SourceFile(const char (&arr)[N])
            : data_(arr)
            , size_(N-1)
        {
            const char* slash = strrchr(data_, '/'); // strrchr is a builtin function
            if (slash)
            {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }

        explicit SourceFile(const char* filename)
            : data_(filename)
        {
            const char* slash = strrchr(filename, '/');
            if (slash)
            {
                data_ = slash + 1;
            }
            size_ = static_cast<int>(strlen(data_));
        }

        const char* data_;
        int size_;
    }; // class SourceFile

    //constructor, destructor
    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool toAbort);
    ~Logger();

    // get stream object
    inline LogStream& stream()
    {
        return impl_.stream_;
    }

    typedef void (*OutputFunc)(const char* msg, int len);
    typedef void (*FlushFunc)();

    // static configurations
    static LogLevel getLogLevel();
    static void setLogLevel(LogLevel level);
    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);
    static void setTime(const Time& tz);

private:
    // private internal class, the impl
    class Impl
    {
    public:
        typedef Logger::LogLevel LogLevel;
        Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
        void formatTime();
        void finish();

        Timestamp time_;
        LogStream stream_;
        LogLevel level_;
        int line_;
        SourceFile basename_;
    }; // class Impl

    Impl impl_;

}; // class Logger

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::getLogLevel()
{
    return g_logLevel;
}

#define LOG_TRACE if (Logger::getLogLevel() <= Logger::TRACE) \
    Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (Logger::getLogLevel() <= Logger::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::getLogLevel() <= Logger::INFO) \
    Logger(__FILE__, __LINE__, Logger::INFO, __func__).stream()
#define LOG_WARN if (Logger::getLogLevel() <= Logger::WARN) \
    Logger(__FILE__, __LINE__, Logger::WARN, __func__).stream()
#define LOG_ERROR if (Logger::getLogLevel() <= Logger::ERROR) \
    Logger(__FILE__, __LINE__, Logger::ERROR, __func__).stream()
#define LOG_FATAL if (Logger::getLogLevel() <= Logger::FATAL) \
    Logger(__FILE__, __LINE__, Logger::FATAL, __func__).stream()

#define LOG_SYSERR Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).stream()


const char* strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor
// initializer lists.

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr)
{
    if (ptr == NULL)
    {
        Logger(file, line, Logger::FATAL).stream() << names;
    }
    else
    {
        return ptr;
    }
}

#define CHECK_NOTNULL(val) \
    CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_LOGGING_H
