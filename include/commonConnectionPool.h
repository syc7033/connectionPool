#pragma once
#include <mymuduo/noncopyable.h>
#include <string>
#include <queue>
#include <connection.h>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>

using namespace std;
/**
 * 实现连接池功能模块
 */
// extern bool g_running;

class ConnectionPool
{
public:
    // 连接池单例模式的全局访问结点
    static ConnectionPool* getInstance();

    // 给外部提供接口 从连接池中获取一个可用的空闲连接
    shared_ptr<Connection> getConnection();
private:
    // 单例模式 构造函数私有化
    ConnectionPool();    
    
    // 加载配置文件 给连接池的一些参数赋值   
    bool loadConfgFile();

    // 运行在独立线程中 => 生产者线程负责 生产新连接
    void produceConnTask();

    // 定时线程 多余连接的回收线程
    void scannerConnTask();

    string _ip;             // mysql的ip地址
    int _port;         // mysql的端口号 默认 3306
    string _username;       // mysql的用户名
    string _password;       // mysql的密码
    string _dbname;         // 数据库名称
    int _initSize;          // 连接池的初始连接量
    int _maxSize;           // 连接池的最大连接量
    int _maxIdletime;       // 连接池的最大空闲时间
    int _connectionTimeout; // 连接池获取连接的超时时间

    queue<Connection*> _connQue;    // 存储mysql连接的队列
    mutex _queMtx;                  // 维持连接队列的线程安全的互斥锁
    condition_variable _queCond;    // 维护连接队列的线程安全的条件变量
    atomic_int _connCnt;            // 创建连接的总数量 线程安全的
};