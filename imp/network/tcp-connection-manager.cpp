#include "tcp-connection-manager.h"
#include "imp/debugger/timer-escaper.h"
using namespace aimy;
TcpConnection::TcpConnection(TaskScheduler *_parent,SOCKET _fd):Object(_parent),disconnected(this),notifyFrame(this),scheduler(_parent)
  ,fd(_fd),peerHostName(NETWORK_UTIL::get_peer_ip(fd)),peerPort(NETWORK_UTIL::get_peer_port(fd)),channel(nullptr),readCache(nullptr),writeCache(nullptr)
{
    channel=scheduler->addChannel(fd);
    channel->bytesReady.connect(this,std::bind(&TcpConnection::hand_recv,this));
    channel->writeReady.connect(this,std::bind(&TcpConnection::hand_write,this));
    channel->closeEvent.connect(this,std::bind(&TcpConnection::hand_close,this));
    channel->errorEvent.connect(this,std::bind(&TcpConnection::hand_error,this));
}

TcpConnection::~TcpConnection()
{
    if(channel)
    {
        if(channel->getFd()!=INVALID_SOCKET)
        {
            channel->stop();
            NETWORK_UTIL::close_socket(channel->getFd());

        }
        channel.reset();
    }
    AIMY_WARNNING("release tcp connection %d",fd);
}

void TcpConnection::setProtocal(std::shared_ptr<ProtocalBase>_protocal)
{
    if(!_protocal)return;
    return invoke(Object::getCurrentThreadId(),[=](){

        channel->stop();
        readCache.reset(new IcacheBufferStream(_protocal,fd));
        writeCache.reset(new IcacheBufferStream(_protocal,fd));
        channel->enableReading();
        channel->sync();
    });
}

bool TcpConnection::sendFrame(const void *frame,uint32_t frame_len)
{

    if(frame_len==0)return true;
    std::shared_ptr<uint8_t>frame_copy(new uint8_t[frame_len+1],std::default_delete<uint8_t[]>());
    memset(frame_copy.get(),0,frame_len+1);
    memcpy(frame_copy.get(),frame,frame_len);
    return invoke(Object::getCurrentThreadId(),[=](){

        auto ret=writeCache->appendFrame(frame_copy.get(),frame_len);
        if(writeCache->frame_count()==1)
        {
            channel->enablWriting();
            channel->sync();
        }
        return ret>0&&ret==frame_len;
    });
}

void TcpConnection::on_recv()
{
    while(1)
    {
        auto ret=readCache->readFromFd();
        if(ret<0)break;
        else if (ret==0) {
            AIMY_DEBUG("recv remote maybe disconnected,close it!");
            on_close();
            break;
        }
        //
        while(1)
        {
            auto frame=readCache->popFrame();
            if(frame.second>0)
            {
                notifyFrame(frame.first,frame.second);
            }else {
                break;
            }
        }
    }
}

void TcpConnection::on_write()
{

    while(writeCache->frame_count()>0)
    {
        auto ret=writeCache->sendCacheByFd();
        if(ret<0)break;
        if(ret==0)
        {
            AIMY_DEBUG("write remote maybe disconnected,close it!");
            on_close();
            return;
        }
    }
    if(writeCache->frame_count()==0)
    {
        channel->disableWriting();
        channel->sync();
    }
}

void TcpConnection::on_close()
{
    AIMY_ERROR("tcp connection closed,disconnect it! [%s]",strerror(NETWORK_UTIL::get_socket_error(fd)));
    channel->stop();
    disconnected();

}

void TcpConnection::on_error()
{
    AIMY_ERROR("tcp connection error,disconnect it! [%s]",strerror(NETWORK_UTIL::get_socket_error(fd)));
    channel->stop();
    disconnected();
}

TcpConnectionManager::TcpConnectionManager(TaskScheduler *parent):Object(parent),scheduler(parent)
{

}

TcpConnectionManager::~TcpConnectionManager()
{

}

bool TcpConnectionManager::addConnection(std::shared_ptr<TcpConnection>connection)
{
    return invoke(Object::getCurrentThreadId(),[=](){
        bool ret=false;
        do{
            if(!connection||connection->getFd()<0)break;
            auto iter=connetions.find(connection->getFd());
            if(iter!=connetions.end())break;
            AIMY_DEBUG("add connection %d",connection->getFd());
            connetions.emplace(connection->getFd(),connection);
        }while(0);
        return ret;
    });
}

void TcpConnectionManager::removeConnection(SOCKET fd)
{
    invoke(Object::getCurrentThreadId(),[=](){
        auto iter=connetions.find(fd);
        if(iter!=connetions.end())
        {
            AIMY_DEBUG("remove connection %d refcount %lu",fd,iter->second.use_count());
            connetions.erase(iter);
        }
    });
}
