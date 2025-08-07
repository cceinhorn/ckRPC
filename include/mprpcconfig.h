#pragma once

#include <unordered_map>
#include <string>
// rpcserver_ip=   rpcserver_port=     zookeeper_ip=   zookeeper_port=
// 框架读取配置文件类
class MprpcConfig
{
public:
    // 解析加载配置文件
    void LoadConfigFile(const char*config_file);
    // 查询配置项信息
    std::string Load(const std::string &key); 
private:
    // 不需要考虑线程安全   整个服务只初始化一次
    std::unordered_map<std::string, std::string> m_configMap;
    // 去掉字符串前后的空格
    void Trim(std::string &src_buf);
};