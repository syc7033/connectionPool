#include <iostream>
#include "commonConnectionPool.h"

using namespace std;
int main()
{
    clock_t begin = clock();
    ConnectionPool *cp = ConnectionPool::getInstance();

    for(int i = 0; i < 100; ++i)
    {
        Connection conn;
        char sql[1024] = {0};
        sprintf(sql, "insert into users(username, age, sex) values('%s', %d, '%s')", "syc", 20, "male");
        shared_ptr<Connection> sp = cp->getConnection();
        // cout << sql << endl;
        sp->update(sql);
    }
    clock_t end = clock();
    cout << (end - begin) << "ms" << endl;
    return 0;
}