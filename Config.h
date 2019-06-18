#ifndef KKWB_CONFIG_H
#define KKWB_CONFIG_H
#include <map>
#include <string>
/*
    配置文件解析
        events
            线程数
            每个线程最大连接数
            日记存放路径
            muti_accept 是否允许一次接收多个连接
            事件驱动模型选择(select、epoll)

        http
            连接超时时间
            单连接请求数上限 keepalive_requests
            单ip请求上限 ip_requests
            MIME类型设置
            sendfile
            server虚拟主机
    SSL
    日记

*/
namespace kkwb
{
    class Config
    {
        public:
            typedef std::map<std::string,std::string> CONFS_MAP;
            typedef std::map<std::string,CONFS_MAP> SERVER_CONFS_MAP;
            void setConfigFile(const std::string& url);
            void readConfig();
        private:
            std::string configFile_;
            CONFS_MAP confs_;
            SERVER_CONFS_MAP serverConfs_;

    };
}


#endif