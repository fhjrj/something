#include "ConnectionMysql.cpp"
#include "MysqlPool.cpp"
#include <string>
#include <iostream>
#include <ctime>
std::mutex m_lock;
auto g=MysqlPool::getPool();

void text()
{
   for (int i=0;i<1000;i++)
{  
    std::unique_lock<std::mutex> locker(m_lock);
    auto  p=g->getMysql();
    std::string sql("insert into user values(1,2)");
    p->update(sql);
}
}
int main()
{
    clock_t begin=clock();
/*
for (int i=0;i<1000;i++)
{
    auto  p=g->getMysql();
    std::string sql("insert into user values(1,2)");
    p->update(sql);
}//20w时间单位上下 单线程使用连接池

*/
std::thread m1(text);
std::thread m2(text);
std::thread m3(text);
m1.join();
m2.join();
m3.join();
//三线程并发使用连接池 50w时间单位上下

/*
for (int i=0;i<1000;i++)
{
     MYSQLConnection P("127.0.0.1","chat","root","123456",3306);
     P.Mysql_connection();
     std::string sql("insert into user values(1,2)");;
     P.update(sql);
}// 单线程6003132时间单位
*/
clock_t end=clock();
std::cout<<end-begin<<std::endl;
}