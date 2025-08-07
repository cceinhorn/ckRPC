#include "logger.h"

#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <time.h>
#include <iostream>


Logger::Logger() 
{   
    //启动专门写日志的线程
    std::thread writeLogTask([&](){
        for (;;)
        {
            // 获取当天的日期，然后取日志信息，写入相应的文件中 a+
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);

            FILE *pf = fopen(file_name, "a+");
            if (pf == nullptr)
            {
                std::cout << "Logger file : " << file_name << "open error!" << std::endl;
                LOG_ERR("Logger file : %s open error!", file_name);
                exit(EXIT_FAILURE);
            }

            std::string msg = m_lckQue.Pop();

            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d => [%s]", 
                                        nowtm->tm_hour, 
                                        nowtm->tm_min, 
                                        nowtm->tm_sec, 
                                        m_loglevel == INFO ? "INFO" : "ERROR");
            msg.insert(0, time_buf);
            msg.append("\n");

            fputs(msg.c_str(), pf);
            fclose(pf);
        }
    });
    // 设置分离线程，守护线程
    writeLogTask.detach();
}

Logger& Logger::GetLoggerInstance()
{
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(LogLevel level)
{
    m_loglevel = level;
}

void Logger::Log(std::string msg)
{
    m_lckQue.Push(msg);
}