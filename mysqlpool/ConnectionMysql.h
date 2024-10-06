#ifndef CONN_MYSQL
#define CONN_MYSQL

#include <string>
#include <mysql/mysql.h>
#include <iostream>
#include <chrono>
 class MYSQLConnection 
 {
    public:
   explicit MYSQLConnection(const std::string host,const std::string db,
           const std::string usr,const std::string password,int port);
    ~MYSQLConnection();
    bool  Mysql_connection();
    bool update(std::string);
    void qurey(std::string);
    void refreshAliveTime(){
      alivetime=std::chrono::steady_clock::now();
    }
   long long getAliveTime()
    {
    std::chrono::nanoseconds res = std::chrono::steady_clock::now() - alivetime;
    std::chrono::milliseconds millisec = std::chrono::duration_cast<std::chrono::milliseconds>(res);
    return millisec.count();
    }
    
    void freeResult();
   private:
   MYSQL* conn_;
   MYSQL_RES* myres;
   int port;
   const std::string db_;
   const std::string usr_;
   const std::string password_; 
   const std::string host_;
   std::chrono::steady_clock::time_point alivetime;
 };

#endif
