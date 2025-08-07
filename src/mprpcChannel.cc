#include "mprpcChannel.h"
#include "logger.h"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <error.h>
#include <arpa/inet.h>
#include <unistd.h>
/*
    header_size + service_name   method_name  args_size + args
*/

// 所有通过stub代理对象调用的rpc方法，都走到这里，统一做rpc方法调用的数据序列化和网络发送
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                                google::protobuf::RpcController* controller, 
                                const google::protobuf::Message* request,
                                google::protobuf::Message* response, 
                                google::protobuf::Closure* done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    // 获取参数的序列化字符串长度
    int args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else 
    {
        controller->SetFailed("Serialize request error!");
        LOG_ERR("Serialize request error!");
        return;
    }

    // 定义Rpc的header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else 
    {
        controller->SetFailed("Serialize rpc header error!");
        LOG_ERR("Serialize rpc header error!");
        return;
    }

    // 组织待发送的rpc请求字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4));    // header_size
    send_rpc_str += rpc_header_str; //rpcheader
    send_rpc_str += args_str;   //args

    // 打印调试信息
    std::cout << "=========================================" << std::endl;
    std::cout << "header_szie: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_size: " << args_size << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "=========================================" << std::endl;
    LOG_INFO("header_szie: %d", header_size);
    LOG_INFO("rpc_header_str: %s", rpc_header_str.c_str());
    LOG_INFO("service_name: %s", service_name.c_str());
    LOG_INFO("method_name: %s", method_name.c_str());
    LOG_INFO("args_size: %d", args_size);
    LOG_INFO("args_str: %s", args_str.c_str());

    // 使用tcp编程，完成rpc方法的远程调用
    int clientfd = socket(AF_INET,SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "Creat socket error! error: %d", errno);
        controller->SetFailed(errtxt);
        LOG_ERR("%s", errtxt);
        return;
    }

    // 读取配置文件rpcserver的信息
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    ZkClient zkCli;
    zkCli.Start();
    std::string method_path = "/" + service_name + "/" + method_name;
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + " is not exist!");
        LOG_ERR("%s is not exist!", method_path.c_str());
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        LOG_ERR("%s address is invalid!", method_path.c_str());
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接rpc服务节点
    if (-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "Connect error! errno: %d", errno);
        controller->SetFailed(errtxt);
        LOG_ERR("%s", errtxt);
        return;
    }

    // 发送rpc请求
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "Send error! errno: %d", errno);
        controller->SetFailed(errtxt);
        LOG_ERR("%s", errtxt);
        return;
    }

    //接受rpc请求的响应值
    char recv_buf[512] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "Recv error! errno: %d", errno);
        controller->SetFailed(errtxt);
        LOG_ERR("%s", errtxt);
        return;
    }

    // std::string response_str(recv_buf, 0, recv_size);    // bug出现问题，recv_buf中遇\0后面的数据就存不下来

    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        close(clientfd);
        char errtxt[1024] = {0};
        sprintf(errtxt, "Parse error! response_str: %s", recv_buf);
        controller->SetFailed(errtxt);
        LOG_ERR("%s", errtxt);
        return;
    }

    close(clientfd);
}