#pragma once

#include "lockqueue.h"

enum LogLevel
{
    INFO,
    ERROR,
};

// MpRpc框架提供的日志系统
class Logger
{
public:
    static Logger& GetLoggerInstance();

    void setLogLevel(LogLevel level);
    void Log(std::string msg);
private:
    int m_loglevel; // 记录日志级别
    LockQueue<std::string> m_lckQue;    //日志缓存队列

    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
};

// 定义宏
#define LOG_INFO(logmsgformat, ...) \
    do  \
    {   \
        Logger &logger = Logger::GetLoggerInstance(); \
        logger.setLogLevel(INFO);   \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__);   \
        logger.Log(c);  \
    } while (0)    

#define LOG_ERR(logmsgformat, ...) \
do  \
{   \
    Logger &logger = Logger::GetLoggerInstance(); \
    logger.setLogLevel(ERROR);   \
    char c[1024] = {0}; \
    snprintf(c, 1024, logmsgformat, ##__VA_ARGS__);   \
    logger.Log(c);  \
} while (0) 
