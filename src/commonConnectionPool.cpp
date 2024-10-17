#include "commonConnectionPool.h"

#include <mymuduo/Logger.h>
#include <stdio.h>
#include <iostream>

using namespace std;
// 线程安全的懒汉单例函数接口
bool g_running = true;

ConnectionPool* ConnectionPool::getInstance()
{
    static ConnectionPool instance; // 静态局部变量的初始化时线程安全的 且只能初始化一次
    return &instance;
}

// 加载配置文件 给连接池的一些参数赋值   
bool ConnectionPool::loadConfgFile()
{
    FILE *pf = fopen("/home/syc/CommonConnectionPool/mysql.conf", "r");
    if(pf == NULL)
    {
        LOG_INFO("mysql.conf file is not exist!");
        return false;
    }

    while(!feof(pf))
    {
        char line[1024] = {0};
        fgets(line, 1024, pf);

        string str = line;
        int index = str.find('=', 0);

        if(index == -1)
        {
            // 无效的配置项
            LOG_INFO("mysql.conf exists invaild conf item!");
            cout << "1" << str << endl;
            continue;
        }
        int endindex = str.find('\n', index);
        string key = str.substr(0, index);
        string value = str.substr(index + 1, endindex - index - 1);

        if(key == "ip")
        {
            _ip = value;
        }
        else if(key == "port")
        {
            _port = atoi(value.c_str());
        }
        else if(key == "username")
        {
            _username = value;
        }
        else if(key == "password")
        {
            _password = value;
        }
        else if(key == "dbname")
        {
            _dbname = value;
        }
        else if(key == "initSize")
        {
            _initSize = atoi(value.c_str());
        }
        else if(key == "maxSize")
        {
            _maxSize = atoi(value.c_str());
        }
        else if(key == "maxIdletime")
        {
            _maxIdletime = atoi(value.c_str());
        }
        else if(key == "connectionTimeout")
        {
            _connectionTimeout = atoi(value.c_str());
        }
    }
    return true;
}

// 连接池的构造函数
ConnectionPool::ConnectionPool()
{
    // 加载配置文件
    if(!loadConfgFile())
    {
        return;
    }

    // 创建初始数量的连接
    for(int i = 0; i < _initSize; i++)
    {
        Connection *conn = new Connection();
        conn->connect(_ip, _username, _password, _dbname);
        conn->refreshAliveTime(); // 刷新一下开始空闲的起始时间
        _connQue.push(conn);
        _connCnt++;
    }
    
    // 启动一个新的线程 作为连接的生产者
    /* thread对象的线程函数如果是成员方法必须用bind绑定器给他绑定一个对象 */
    thread produceConnTask(std::bind(&ConnectionPool::produceConnTask, this));
    produceConnTask.detach();

    // 启动一个定时线程 扫描超过maxIdleTime的空闲连接 进行连接回收
    thread scannerConnTask(std::bind(&ConnectionPool::scannerConnTask, this));
    scannerConnTask.detach();

    // 把上面俩个线程设置为分离线程

} 

// 生产者线程函数
void ConnectionPool::produceConnTask()
{
    while(g_running)
    {
        unique_lock<mutex> lock(_queMtx);
        while(!_connQue.empty())
        {
            _queCond.wait(lock); // 队列不空 生产线程等待消费线程去消费
        }

        // 连接数量没有到达上线 继续创建新的连接
        if(_connCnt < _maxSize)
        {
            Connection *conn = new Connection();
            conn->connect(_ip, _username, _password, _dbname);
            conn->refreshAliveTime();
            _connQue.push(conn);
            _connCnt++;
        }

        // 如果没到达上线 生产完了肯定要通知 到达上线了更要通知 
        _queCond.notify_all();
    }
}

// 给外部提供接口 从连接池中获取一个可用的空闲连接 消费者线程
shared_ptr<Connection> ConnectionPool::getConnection()
{
    unique_lock<mutex> lock(_queMtx);
    while(_connQue.empty())
    {
        if(cv_status::timeout == _queCond.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
        {
            // 被唤醒 俩种可能 #1 生产者生产完了 给我唤醒了 #2 超时了
            if(_connQue.empty())
            {
                LOG_INFO("获取空闲连接超时...获取连接失败");
                return nullptr;
            }
        }
    }
    /**
     * shared_ptr智能指针析构时，会直接把connection析构掉了
     * 相当于调用了connection的析构函数 conn直接被close掉了
     * 我们想的是智能指针析构之后把conn归还给连接池的队列中
     * 所有这里需要自定义shared_ptr的释放资源方式
     */
    shared_ptr<Connection> sp(_connQue.front(), [&](Connection *backConn){
        // 这里是在服务器应用线程中调用的 需要考虑队列的线程安全问题
        unique_lock<mutex> lock(_queMtx);
        backConn->refreshAliveTime();
        _connQue.push(backConn);
    });
    _connQue.pop();
    _queCond.notify_all();
    // cout << 1 << endl;
    return sp; 
}

// 定时线程 多余连接的回收线程
void ConnectionPool::scannerConnTask()
{
    while(g_running)
    {
        // 通过sleep模拟定时效果
        this_thread::sleep_for(chrono::seconds(_maxIdletime));

        // 扫描整个队列 释多余的连接
        unique_lock<mutex> lock(_queMtx);
        while(_connCnt > _initSize)
        {
            Connection *conn = _connQue.front();
            if(conn->getAliveTime() >= (_maxIdletime * 1000))
            {
                _connQue.pop();
                _connCnt--;
                delete conn; // 调用connection的析构函数 释放连接
            }
            else 
            {
                // 对头存活时间都没有超过最大空闲时间 后面的也不用看了
                break;
            }
        }
    }
}