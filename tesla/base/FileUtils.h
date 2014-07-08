/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/17/2014
 * Description
 *     File utility classes
 *
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_FILEUTILS_H
#define TESLA_BASE_FILEUTILS_H

#include <tesla/base/StringPiece.hpp>
#include <tesla/base/Noncopyable.hpp>

namespace tesla
{

namespace base
{

// read small file < 64KB
class SmallFileReader
    : private Noncopyable
{
public:
    SmallFileReader(StringArg filename);
    ~SmallFileReader();

    // return errno
    template<typename String>
    int readToString(int maxSize,
                     String* content,
                     int64_t* fileSize,
                     int64_t* modifyTime,
                     int64_t* createTime);

    /// Read at maxium BUFFER_SIZE into buf_
    // return errno
    int readToBuffer(int* size);

    const char* buffer() const
    {
        return buf_;
    }

    static const int BUFFER_SIZE = 64*1024;

private:
    int fd_;
    int err_;
    char buf_[BUFFER_SIZE];
}; // class SmallFileReader

// read the file content, returns errno if error happens.
template<typename String>
int readFile(StringArg filename,
             int maxSize,
             String* content,
             int64_t* fileSize = NULL,
             int64_t* modifyTime = NULL,
             int64_t* createTime = NULL)
{
    SmallFileReader file(filename);
    return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
}

// not thread safe
class FileAppender
    : private Noncopyable
{
public:
    explicit FileAppender(StringArg filename);

    ~FileAppender();

    void append(const char* logline, const size_t len);

    void flush();

    size_t writtenBytes() const
    {
        return writtenBytes_;
    }

private:

    size_t write(const char* logline, size_t len);

    FILE* fp_;
    char buffer_[64*1024];
    size_t writtenBytes_;
}; // class FileAppender

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_FILEUTILS_H
