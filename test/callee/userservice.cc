#include <cstdint>
#include <iostream>
#include <string>

#include "user.pb.h"
#include "mprpcapplication.h"
#include "mprpcprovider.h"

/*
本地服务，提供进程内的本地方法
*/
class UserService : public fixbug::UserServiceRpc      // RPC服务发布端        UserServiceRpc_stub给caller使用的    UserServiceRpc_stub/UserServiceRpc框架
{
public:
    // 本地业务
    bool Login(std::string name, std::string pwd)
    {   
        std::cout << "Doing local service: Login" << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service: Register" << std::endl;
        std::cout << "id:" << id << "name:" << name << "pwd:" << pwd << std::endl;
        return true;
    }

    // 重写基类虚函数   下面functions都是框架直接调用的
    /*
    1. caller  ==>  Login(LoginRequest)   ==>   muduo   ==> callee
    2. callee   ==> Login(LoginRequest)   ==>   对应的Login处理
    */
    void Login(::google::protobuf::RpcController* controller,       // 反序列化
                       const ::fixbug::LoginRequest* request,       // 封装的请求数据
                       ::fixbug::LoginResponse* response,           // 处理之后回调
                       ::google::protobuf::Closure* done)           // 序列化返回
    {
        //  框架给业务上报了请求参数LoginRequest，应用业务获取相应数据做本地业务
        /*
        理解：
        1. 相比于协议的解析，数据封装到对象中传输更方便
        2. 所以调用远端服务并不是将服务请求到本地处理，而是将数据传到远端处理完并返回给本地，造成本地处理的假象？
        3. 获取数据 => 本地处理 => 响应写入 => 序列化返回   request => 本地 => response => done
        */
        std::string name = request->name();
        std::string pwd = request->pwd();
        // 本地业务Login()
        bool login_result = Login(name, pwd); 

        // 把响应写入 包括：错误码  错误消息    返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);
        
        // 执行回调操作     执行具体派生的Closure类中Run()方法，执行响应对象数据的序列化和网络发送（框架处理完成）
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,       // 反序列化
                    const ::fixbug::RegisterRequest* request,       // 封装的请求数据
                    ::fixbug::RegisterResponse* response,           // 处理之后回调
                    ::google::protobuf::Closure* done)           // 序列化返回
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();
        // 本地业务Login()
        bool registern_result = Register(id, name, pwd); 

        // 把响应写入 包括：错误码  错误消息    返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(registern_result);
        
        // 执行回调操作     执行具体派生的Closure类中Run()方法，执行响应对象数据的序列化和网络发送（框架处理完成）
        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 调用框架的初始化操作     provider -i config.conf  
    MprpcApplication::Init(argc, argv);

    // 把UserService对象发布到rpc节点上     provider是一个rpc网络服务对象   要做到高并发
    RpcProvider provider;
    provider.NotifyService(new UserService());

    //启动一个rpc服务发布节点   Run以后 进程进入阻塞状态    等待远程rpc调用请求
    provider.Run();

    return 0;
}