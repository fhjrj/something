#include "ConnectionMysql.h"

 MYSQLConnection::MYSQLConnection(const std::string host,const std::string db,
           const std::string usr,const std::string password,int port=0):
 db_(db),
 usr_(usr),
 password_(password),
 port(port),
 host_(host),
 myres(nullptr)
 {
    conn_=mysql_init(nullptr);
    if(!conn_)
    {
      std::cout<<"init :"<<mysql_error(conn_)<<std::endl;
    }
 }

void MYSQLConnection::freeResult() {
    if (myres) {
        mysql_free_result(myres);
        myres= nullptr;
    }
}

 MYSQLConnection::~MYSQLConnection(){
    if(conn_)
    {
        mysql_close(conn_);
    }
    freeResult();//清空残留数据
 }

 bool MYSQLConnection::Mysql_connection()
 {
   MYSQL* p=mysql_real_connect(conn_,host_.c_str(),usr_.c_str(),password_.c_str(),db_.c_str(),port,nullptr,0);
   if(p==nullptr){
      std::cout<<"connection:"<<mysql_error(conn_)<<std::endl;
       return false;
   }
   return true;
 }

 bool MYSQLConnection::update(std::string qury)
 {
    if(mysql_query(conn_,qury.c_str()))
    {     
           std::cout<<"update:"<<mysql_error(conn_)<<std::endl;
           return false;
    }
    return true;
 }//update

void MYSQLConnection::qurey(std::string qury)
 {
   freeResult();
 if(mysql_query(conn_,qury.c_str()))
    {     
   std::cout<<"qurey:"<<mysql_error(conn_)<<std::endl;
        return;
    }
    myres=mysql_store_result(conn_);
 }//select

 

