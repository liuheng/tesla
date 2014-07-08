/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/18/2014
 * Description
 *     process and thread infos
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_PROCESSINFO_H
#define TESLA_BASE_PROCESSINFO_H

#include <tesla/base/StringPiece.hpp>
#include <tesla/base/Types.hpp>
#include <tesla/base/Timestamp.h>
#include <vector>

namespace tesla
{

namespace base
{

class ProcessInfo
{
public:
    static pid_t pid();
    static string pidString();
    static uid_t uid();
    static string username();
    static uid_t euid();
    static Timestamp startTime();
    static int clockticksPerSecond();
    static int pagesize();
    static bool debugBuild();  // constexpr

    static string hostName();
    static string procName();
    static StringPiece procName(const string& stat);

    // read /proc/self/status
    static string procStatus();

    // read /proc/self/stat
    static string procStat();

    // read /proc/self/task/tid/stat
    static string threadStat();

    // readlink /proc/self/exe
    static string exePath();

    static int openedFiles();
    static int maxOpenFiles();

    struct CpuTime
    {
        double userSeconds;
        double systemSeconds;

        CpuTime() : userSeconds(0.0), systemSeconds(0.0) { }
    };
    static CpuTime cpuTime();

    static int numOfThreads();

    static std::vector<pid_t> threads();
}; // class ProcessInfo

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_PROCESSINFO_H
