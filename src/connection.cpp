#include "connection.h"

#include <mymuduo/Logger.h>



// 初始化数据库连接
Connection::Connection()
{
    _conn = mysql_init(nullptr);
}

// 释放数据库的连接资源
Connection::~Connection()
{
    if(_conn != nullptr) mysql_close(_conn);
}
// 连接数据库
bool Connection::connect(string ip, string username, string pwd, string dbname)
{
    MYSQL *p = mysql_real_connect(
        _conn, ip.c_str(),username.c_str(), 
        pwd.c_str(), dbname.c_str(), 
        3306, nullptr, 0
    );
    if(p != nullptr)
    {
        // C和C++代码默认字符集时ASCII 如果不设置 从MSYQL上拉取下来的中文显示会乱码？
        mysql_query(_conn, "set names gbk");
        // LOG_INFO("connect mysql sucess!");
    }
    else
    {
        LOG_INFO("connect mysql fail!");
    }
    return p;
}

// 更新操作
bool Connection::update(string sql)
{
    if(mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO("%s:%d:%s 更新失败", __FILE__, __LINE__, sql.c_str());
        return false;
    }
    return true;
}

// 查询操作
MYSQL_RES* Connection::query(string sql)
{
    if(mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO("%s:%d:%s 查询失败", __FILE__, __LINE__, sql.c_str());
        return nullptr;
    }
    return mysql_use_result(_conn);
}

// 获取连接
MYSQL* Connection::getConnection()
{
    return _conn;
}