#pragma once

#include "mprpcconfig.h"
#include "mprpcChannel.h"
#include "mprpccontroller.h"
#include "mprpcconfig.h"

// mprpc框架的base类
class MprpcApplication
{
public:
    static void Init(int argc, char **argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();
private:
    static MprpcConfig m_config;

    // 可定义变量记录是否初始化，框架只初始化一次       单例模式
    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
};