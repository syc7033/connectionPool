#pragma once
#include <mysql/mysql.h>
#include <string>
#include <ctime>

using namespace std;


/**
 * 描述数据库的一次连接 
 * connect update query getConnection
 */
// 数据库数据类
class Connection
{
public:
    // 初始化数据库连接
    Connection();
    // 释放数据库的连接资源
    ~Connection();
    // 连接数据库
    bool connect(string ip, string username, string pwd, string dbname);
    // 更新操作
    bool update(string sql);
    // 查询操作
    MYSQL_RES* query(string sql);
    // 获取连接
    MYSQL* getConnection();
    // 刷新连接在队列中的空闲时间
    void refreshAliveTime() { _alivatime = clock(); } 
    clock_t getAliveTime() const { return clock() - _alivatime; }
private:
    MYSQL *_conn; // 表示和MySQL Server的每次连接
    clock_t _alivatime; // 记录进入队列的时间
};