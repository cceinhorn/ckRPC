#include <cstdint>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <vector>

// #include "user.pb.h"
#include "mprpcapplication.h"
#include "mprpcprovider.h"
#include "friend.pb.h"
#include "logger.h"

class FriendService : public fixbug::FriendsServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(uint32_t userid)
    {
        std::cout << "do GetFriendsList Service! userid: " << userid << std::endl;
        std::vector<std::string> vec;
        vec.push_back("chen kun");
        vec.push_back("ai ai");
        return vec;
    }

    void GetFriendsList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendsListRequest* request,
                       ::fixbug::GetFriendsListResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t userid = request->userid();
        std::vector<std::string> friendList = GetFriendsList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for (std::string &name : friendList) 
        {
            std::string *p = response->add_friends();
            *p = name;
        }

        done->Run();
    }
};


int main(int argc, char **argv)
{   
    // LOG_INFO("first log message!");
    // LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);

    // 调用框架的初始化操作     provider -i config.conf  
    MprpcApplication::Init(argc, argv);

    // 把UserService对象发布到rpc节点上     provider是一个rpc网络服务对象   要做到高并发
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    //启动一个rpc服务发布节点   Run以后 进程进入阻塞状态    等待远程rpc调用请求
    provider.Run();

    return 0;
}