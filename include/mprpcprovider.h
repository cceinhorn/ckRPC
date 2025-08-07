#pragma once
#include "mprpcprovider.h"
#include "mprpcapplication.h"
#include "mprpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
 

// 框架提供的专门发布rpc服务的网络对象类
class RpcProvider
{
public:
    // 这里是框架提供给外部使用的，可以发布rpc方法的接口
    void NotifyService(google::protobuf::Service *Service);

    // 启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();

private:
  // 组合EventLoop
  muduo::net::EventLoop m_eventLoop;

  // service服务类型信息
  struct ServiceInfo
  {
    // 保存服务对象
    google::protobuf::Service *m_service;  
    // 保存服务方法
    std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap; 
  };
  // 存储注册成功的服务对象和其服务方法的所有信息   抽象层
  std::unordered_map<std::string, ServiceInfo> m_serviceMap;

  // 新的Socket连接回调
  void OnConnection(const muduo::net::TcpConnectionPtr&);
  // 已建立连接用户的读写事件回调
  void OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);
  // Closure回调操作， 用于序列化rpc的响应和网络发送
  void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
};