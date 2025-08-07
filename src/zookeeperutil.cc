#include "zookeeperutil.h"
#include "logger.h"

#include <cstdlib>
#include <iostream>
#include <semaphore.h>


// 全局的watcher观察器  zkserver给zkclient的通知
void global_watcher(zhandle_t *zh,
                    int type,
                    int state,
                    const char *path,
                    void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)  // 回调的消息类型是和会话相关的消息类型
    {
        if (state == ZOO_CONNECTED_STATE)   // zkclient和zkserver连接成功
        {
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{

}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);
    }
}

// zkclient启动连接zkserver
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    /*
    zookeeper_mt: 多线程
    zookeeper的API客户端提供了三个线程:
    API调用线程
    网络I/O线程 pthread_create poll     定时发送ping心跳消息  1/3 timeout
    watcher回调线程
    */
    // 连接异步 只是初始化成功，内存开辟等  
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr,0);
    if (nullptr == m_zhandle)
    {
        std::cout << "zookeeper_init error!" << std::endl;
        LOG_ERR("zookeeper_init error!");
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);
    
    // 阻塞，等待有响应回调 sem_post(sem)
    sem_wait(&sem);
    std::cout << "zookeeper_init success!" << std::endl;
    LOG_INFO("zookeeper_init success!");
}

// 在zkserver上根据指定path创建znode节点
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int buffer_len = sizeof(path_buffer);
    int flag;

    // 先判断path表示的znode节点是否存在
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (ZNONODE == flag)
    {   
        // 创建指定path的znode节点
        flag = zoo_create(m_zhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, buffer_len);
        if (flag == ZOK)
        {
            std::cout << "znode create success ... path: " << path << std::endl;
            LOG_INFO("znode create success ... path: %s", path);
        }
        else 
        {
            std::cout << "flag: " << flag << std::endl;
            LOG_ERR("flag: %d", flag);
            std::cout << "znode create error ... path" << path << std::endl;
            LOG_ERR("znode create error ... path: %s", path);
            exit(EXIT_FAILURE);
        }
    }
}

// 根据参数指定的znode节点路径，或者znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
    int buffer_len = sizeof(buffer);
    int flag = zoo_get(m_zhandle, path, 0, buffer, &buffer_len, nullptr);
    if (flag != ZOK)
    {
        std::cout << "Get znode error ... path: " << path << std::endl;
        LOG_ERR("Get znode error ... path: %s", path);
        return "";
    }
    else
    {
        return buffer;
    } 
}