#include "MysqlPool.h" 


MysqlPool::~MysqlPool()
{
    if(produce->joinable())
    {
        produce->join();
    }
    if(scanner->joinable())
    {
        scanner->join();
    }
}

std::shared_ptr<MysqlPool> MysqlPool::getPool()
{
    if(ConnectionPool_){
        return ConnectionPool_;
    }
     std::unique_lock<std::mutex> locker(m_mutex);
     if(ConnectionPool_){
        locker.unlock();
        return ConnectionPool_;
    }
    ConnectionPool_=std::shared_ptr<MysqlPool>(new MysqlPool);
     locker.unlock();
    return ConnectionPool_;
}

//配置文件中加载配置项
bool MysqlPool::loadConfigFile() 
{
  FILE* fp=fopen("mysql.ini","r");
  if(!fp){
    return false;
  }
  while(!feof(fp))
  {
    char line[1024]={0};
    fgets(line,1024,fp);
    std::string str(line);
    int idx=str.find('=',0);//  XXXX=XXXX,分割maxSize
    if(idx==-1)//未找到
    {
     continue;
    }
    int endidx=str.find('\n',idx);
    std::string key=str.substr(0,idx);
    std::string value=str.substr(idx+1,endidx-idx-1);
    if(key=="ip") host_=value;
    else if(key=="port") port_=atoi(value.c_str());
    else if(key=="username") usr_=value;
    else if(key=="password") password_=value;
    else if(key=="initSize") initSize_=atoi(value.c_str());
    else if(key=="maxSize") maxSize_=atoi(value.c_str());
    else if(key=="maxIdleTime") maxIdleTime=atoi(value.c_str());
    else if(key=="dbname") db_=value;
    else if(key=="MaxConnectionTimeOut") timeout=atoi(value.c_str());
  }
   return true;
}

void MysqlPool::prodectMysql()
{
       for(;;)
       {
       std::unique_lock<std::mutex> locker(m_mutex);
         while(connectionQueue_.size()>=initSize_)
       {
        m_cond.wait(locker);
       }//当生产队列大于最小数量限制时，才挂起，不需要生产

        if(conn_size.load()<maxSize_)
        {    
            MYSQLConnection* newconn(new MYSQLConnection(host_,db_,usr_,password_,port_));
            newconn->refreshAliveTime();
            conn_size++;
            newconn->Mysql_connection();
            connectionQueue_.push(newconn);
        }
        m_cond.notify_all();
    }
}


std::shared_ptr<MYSQLConnection> MysqlPool::getMysql()//按理是消费队列为空时，才挂起
{
    std::unique_lock<std::mutex> locker(m_mutex);
    while(connectionQueue_.empty())
    {
        if(std::cv_status::timeout==m_cond.wait_for(locker,std::chrono::milliseconds(timeout)))//如果为空 挂起等待一段时间,中途被唤醒 说明有生产者生产或者消费者取出
//生产者生产进行轮训检查
        {
            if(connectionQueue_.empty()||conn_size.load()==0)
        {   
            std::cout<<"get Mysql false"<<std::endl;
            return nullptr;
        }
        }
    }
    std::shared_ptr<MYSQLConnection> sp(connectionQueue_.front(),
    [this](MYSQLConnection* p){
        std::unique_lock<std::mutex> locker(this->m_mutex);
        p->refreshAliveTime();
        this->connectionQueue_.push(p);//使用后重新加入回去，还存在在集合中
    });
    connectionQueue_.pop();
    m_cond.notify_all();//这里进行唤醒是让生产者进行检查。看是否满足生产条件，同时唤醒其他被等待挂起的消费者
    return sp;
}

void MysqlPool::scanner_()
    {
        while(1)
        {   
            std::this_thread::sleep_for(std::chrono::milliseconds(1000*10));
            std::unique_lock<std::mutex> locker(m_mutex);
            while(connectionQueue_.size()>initSize_)
            {    
                MYSQLConnection* p=connectionQueue_.front();//插入都是以绝对时间插入，多以是递增的
                if(p->getAliveTime()>=maxIdleTime)
                {
                connectionQueue_.pop();
                conn_size--;
                delete p;
                p=nullptr;
                }
                else{
                break;
                }
            }
        }
    }

std::shared_ptr<MysqlPool> MysqlPool::ConnectionPool_=nullptr;
std::mutex MysqlPool::m_mutex;