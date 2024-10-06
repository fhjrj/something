#ifndef MYSQL_POOL_
#define MYSQL_POOL_

#include <queue>
#include "ConnectionMysql.h"
#include <condition_variable>
#include <stdio.h>
#include <memory>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <atomic>
#include <iostream>
#include <mutex>
  class MysqlPool
  {
     public:

     MysqlPool():connectionQueue_(),conn_size(0)
     {
         if(loadConfigFile()) 
          std::cout<<"init success"<<std::endl;
         else{
          std::cout<<"init false"<<std::endl;
           return;
           }
    
           for(int i=0;i<initSize_;i++)
           {
            MYSQLConnection* newconn(new MYSQLConnection(host_,db_,usr_,password_,port_));
            newconn->refreshAliveTime();
            conn_size++; 
            newconn->Mysql_connection();
            connectionQueue_.push(newconn);
           }

          produce=std::shared_ptr<std::thread> (new std::thread([this](){
              this->prodectMysql();
          }));

           scanner=std::shared_ptr<std::thread> (new std::thread([this](){
              this->scanner_();
          }));

     }//可用日志代替,线程类可以自定义线程类

     ~MysqlPool();
     static std::shared_ptr<MysqlPool> getPool();
     std::shared_ptr<MYSQLConnection> getMysql();
     private:
     static std::shared_ptr<MysqlPool> ConnectionPool_;
     bool loadConfigFile() ;//配置文件中加载配置
     void prodectMysql();
     void scanner_();
     
     static std::mutex m_mutex;

     int port_;
      std::string db_;
      std::string usr_;
      std::string password_;
      std::string host_;
      std::atomic<int> conn_size;//连接总数量
     int initSize_;
     int maxSize_;//最大连接数量
     int maxIdleTime;//最大空闲时间
     int timeout;//获取连接的超时时间
     std::shared_ptr<std::thread> produce;
     std::shared_ptr<std::thread> scanner;
     std::queue<MYSQLConnection*> connectionQueue_; 
     std::condition_variable m_cond;
     };
//connection操作放在插入队列时就进行连接可以避免mysql的connection出错，而不是用户自己操作
#endif