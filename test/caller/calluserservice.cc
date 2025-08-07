#include <google/protobuf/service.h>
#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcChannel.h"


int main(int argc, char **argv)
{
    // 整个程序启动以后，想使用mprpc框架来享受rpc服务调用，一定需要先调用框架初始化函数一次
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    // rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("chenkun");
    request.set_pwd("123456");
    // rpc方法的响应
    fixbug::LoginResponse response;
    // 发起rpc方法的调用 同步的rpc调用过程  MprpcChannel::callMethod        同步阻塞
    stub.Login(nullptr, &request, &response, nullptr);  // RpcChannel::callMethod 集中来做所有rpc请求

    // 一次rpc调用完成，读调用的节点结果
    if (response.result().errcode() == 0)
    {
        // 成功
        std::cout << "Rpc Login response success:" << response.success() << std::endl; 
    }
    else 
    {
        // 失败
        std::cout << "Rpc Login response error:" << response.result().errmsg() <<std::endl;
    }

    // Register
    fixbug::RegisterRequest registerrequest;
    registerrequest.set_id(2000);
    registerrequest.set_name("chenkun");
    registerrequest.set_pwd("666666");
    fixbug::RegisterResponse registerresponse;
    stub.Register(nullptr, &registerrequest, &registerresponse, nullptr);

    if (registerresponse.result().errcode() == 0)
    {
        // 成功
        std::cout << "Rpc Register response success:" << registerresponse.success() << std::endl; 
    }
    else 
    {
        // 失败
        std::cout << "Rpc Register response error:" << registerresponse.result().errmsg() <<std::endl;
    }

    return 0;
}