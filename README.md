# RPC
本项目在分布式环境下完成本地服务在 RPC 节点上的注册、发布与远程调用；项目构建自定义通信协议与服务注册中心，集成日志系统，设计高并发网络通信模型，具备高性能与良好的扩展性。

# 功能
* 利用protobuf实现数的消息的序列化与反序列化；
* 利用zookeeper作为服务中间件，实现分布式部署中的服务注册与发现；
* 良好的watcher机制监听节点变化；
* 配备自定义的异步日志功能；

# 运行环境
* Ubuntu 20.04.2
* gcc 9.4.0
* cmake 4.0.3

# 库准备
```shell
// 基础工具
sudo apt-get install -y wget cmake build-essential unzip
```
muduo安装需要依赖boost

[boost](https://www.boost.org/releases/latest/)
```shell
sudo apt-get install -y libboost-all-dev
```
[muduo](https://blog.csdn.net/QIANGWEIYUAN/article/details/89023980) 

[protobuf](https://github.com/google/protobuf)
```shell
sudo apt-get install protobuf-compiler libprotobuf-dev
```

[zookeeper](https://zookeeper.apache.org/releases.html)
```shell
sudo apt install zookeeperd
```

## 目录树
```
.
├── bin             可执行文件
│   └── test.conf   ip:port配置文件
├── include         框架头文件
│   └─...
├── src             框架源代码
│   └─...
├── test           
│   ├── callee      服务提供方
│   ├── caller      服务请求方
│   └── .proto      接口及数据结构描述
├── build.sh        编译文件
├── CMakeLists.txt 
├── README.md
├── rpcconsumer.png 请求测试 
└── rpcprovider.png 注册测试
```

# 项目启动
```bash
// 编译
bash build.sh
```
```shell
cd bin
// 开启对应的zookeeper
// 服务提供方注册服务
./provider -i test.conf
// 服务请求方消费服务
./consumer -i test.conf
```

# 测试结果
![testprovider](https://github.com/cceinhorn/ckRPC/blob/master/rpcprovider.png)

![testconsumer](https://github.com/cceinhorn/ckRPC/blob/master/rpcconsumer.png)

# 致谢
[muduo](https://github.com/chenshuo/muduo)

[webserver](https://github.com/markparticle/WebServer)  























Dependencies:

muduo     // https://github.com/chenshuo/muduo

boost     // https://www.boost.org/releases/latest/

protobuf  // https://github.com/protocolbuffers/protobuf

zookeeper // https://zookeeper.apache.org/releases.html
  
