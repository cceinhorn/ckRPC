#include "mprpcprovider.h"

#include <chrono>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <muduo/net/TcpServer.h>
#include <functional>
#include <string>
#include <zookeeper/zookeeper.h>

/*
service_name => service描述
                    => service* 记录服务对象
                    method_name => method的方法对象
json        存储文本信息，具有除数据以外的信息      
protobuf    存储二进制信息，占用带宽更少，数据更紧致，效率更高，相同带宽传输数据更多    可以在抽象层面描述  
*/

// 这里是框架提供给外部使用的，可以发布rpc方法的接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象的方法数量
    int method_count = pserviceDesc->method_count();

    // std::cout << "Service name:" << service_name << std::endl;
    LOG_INFO("Service name: %s", service_name.c_str());

    for (int i = 0; i < method_count; ++i) 
    {
        // 获取服务对象指定下标的服务方法的描述 (抽象描述)
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        // std::cout << "Method name:" << method_name << std::endl;
        LOG_INFO("Method name: %s", method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

// 启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    // 绑定连接回调和消息读写回调方法   分离网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, 
        std::placeholders::_2,std::placeholders::_3));

    //设置muduo库的线程数量     muduo自动分发I/O线程和工作线程      epoll + 多线程
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout 30s  zkclient 网络I/O线程    1/3 * timeout   时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    // service_name为永久性节点     method_name为临时性节点
    for (auto &sp : m_serviceMap)
    {
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_methodMap)
        {
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    // std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl; 
    LOG_INFO("RpcProvider start service at ip: %s, port: %d", ip.c_str(), port);

    // 启动网络服务
    server.start();
    m_eventLoop.loop();
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 和rpc client的连接断开
        conn->shutdown();
    }
}

/*
    在框架内部，RpcProvider和RpcConsumer商量好之间通信用的protobuf数据类型
    service_name method_name args_size    定义proto的message类型，进行数据头的序列化和反序列化
    消息头  参数
    UserServiceLoginzhang  san123456       需要记录长度，有粘包问题
    head_size + header_str + args_str
    4字节
    std::string     insert和copy方法
*/
// 已建立连接用户的读写事件回调 如果远程有一个rpc服务的调用请求 那么OnMessage方法就会回应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp)
{
    // 网络上接收的远程rpc调用请求的字符流      Login   args
    std::string recv_buf = buffer->retrieveAllAsString();

    // 从字符流中读取前4个字节的内容    header_size通过四个字节存储具体的数字，后面获取header_size长度信息
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);

    // 根据header_size读取数据头的原始字符流
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    std::string service_name;
    std::string method_name;
    uint32_t args_size;

    // 反序列化数据，得到rpc请求的详细信息
    mprpc::RpcHeader rpcHeader;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    } 
    else
    {
        // 数据头反序列化失败
        // std::cout << "Rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        LOG_ERR("Rpc_header_str: %s parse error!", rpc_header_str.c_str());
    } 

    // 获取rpc方法参数的字符流
    std::string args_str = recv_buf.substr(4 + header_size, args_size);


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

    // 判断是否存在服务及方法
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end()) 
    {
        // std::cout << service_name << " is not exit!" << std::endl;
        LOG_ERR("%s is not exit!", service_name.c_str());
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        // std::cout << service_name << ":" << method_name << " is not exit" << std::endl;
        LOG_ERR("%s: %s is not exit!", service_name.c_str(), method_name.c_str());
    }

    // 获取service对象和method对象
    google::protobuf::Service *service = it->second.m_service;  
    const google::protobuf::MethodDescriptor *method= mit->second;

    // 生成rpc方法调用的请求request和response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        // std::cout << "Request parse error, content:" << args_str << std::endl;
        LOG_ERR("Request parse error, content: %s", args_str.c_str());
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的method的调用， 绑定一个Closure回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                                    const muduo::net::TcpConnectionPtr&, 
                                                                    google::protobuf::Message*>
                                                                    (this, 
                                                                    &RpcProvider::SendRpcResponse, 
                                                                    conn, response);

    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))
    {
        // 序列化成功后，通过网络把rpc方法执行的结果发送回rpc调用方
        conn->send(response_str);
        // 模拟http的短链接服务，由rpcprovider主动断开连接
    }
    else
    {
        // std::cout << "Serialize response_str error!" << std::endl;
        LOG_ERR("Serialize response_str error!");
    }
    conn->shutdown();
}