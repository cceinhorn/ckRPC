#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"


int main(int argc, char **argv)
{
    MprpcApplication::Init(argc, argv);

    fixbug::FriendsServiceRpc_Stub stub(new MprpcChannel());
    fixbug::GetFriendsListRequest request;
    request.set_userid(22);
    // rpc方法的响应
    fixbug::GetFriendsListResponse response;
    // 发起rpc方法的调用 同步的rpc调用过程  MprpcChannel::callMethod        同步阻塞
    MprpcController controller;
    stub.GetFriendsList(&controller, &request, &response, nullptr);  // RpcChannel::callMethod 集中来做所有rpc请求

    // 一次rpc调用完成，读调用的节点结果
    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (response.result().errcode() == 0)
        {
            // 成功
            std::cout << "Rpc GetFriendsList response success!" << std::endl; 
            int size = response.friends_size();
            for (int i = 0; i < size; ++i)
            {
                std::cout << "index: " << (i + 1) << " name: " <<response.friends(i) << std::endl;
            }
        }
        else 
        {
            // 失败
            std::cout << "Rpc GetFriendsList response error:" << response.result().errmsg() <<std::endl;
        }
    }

    return 0;
}