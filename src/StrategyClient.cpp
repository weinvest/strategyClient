#include "StrategyClient.h"
#include "network/TcpClient.h"
#include <boost/thread.hpp>
#include "common/WBlocker.h"
#include "proto/get_constants.h"
#include "proto/get_types.h"
#include "network/TSerializer.h"
#include "crypto/Crypto.h"
StrategyClient::StrategyClient()
    :mIoService(new boost::asio::io_service())
    ,mWork(new boost::asio::io_service::work(*mIoService))
    ,mConnection(new TcpClient(*mIoService))
    ,mWorkThread(new boost::thread(boost::bind(&boost::asio::io_service::run,mIoService)))
{
}

StrategyClient::~StrategyClient()
{}

StrategyClient::Result StrategyClient::connect(std::string& host,int port)
{
    StrategyClient::Result result;
    bool hasDone = false;
    WBlocker blocker;
    mConnection->OnConnected.connect_extended([&result,&blocker,&hasDone](const boost::signals2::connection& c)
    {
        c.disconnect();
        result.first = true;
        hasDone = true;
        blocker.notify();
    }
    );
    mConnection->OnError.connect_extended([&result,&hasDone,&blocker](const boost::signals2::connection& c
                                          ,const boost::system::error_code& ec)
    {
        c.disconnect();
        hasDone = true;
        result.first = false;
        result.second = boost::system::system_error(ec).what();
        blocker.notify();
    });
    mConnection->connect(host,port);
    blocker.wait([&hasDone](){return hasDone;});
    if(!hasDone)
    {
        result.first = false;
        result.second = "connect timed out";
    }

    return result;
}

StrategyClient::Result StrategyClient::getResource(ResType::type resType
                                                   ,const std::string& userName
                                                   ,const std::string& password
                                                   ,const std::string& tradingDay)
{
    StrategyClient::Result result;
    bool hasDone = false;
    WBlocker blocker;
    mConnection->OnResponse.connect_extended([this,&blocker,&result,&hasDone](const boost::signals2::connection& c
                                             ,WMessage::Ptr pMessage)
    {
        c.disconnect();
        GetResponse response = TSerializer::deserialize<GetResponse>(pMessage);
        result.first = response.Status == GetStatus::Success;
        if(result.first && ResType::Mapping != response.Type)
        {
            AESDecrypt(result.second,response.Key,response.Content,0);
        }
        else
        {
            result.second = response.Content;
        }
        hasDone = true;
        blocker.notify();
    });

    mConnection->OnError.connect_extended([&result,&hasDone,&blocker](const boost::signals2::connection& c
                                          ,const boost::system::error_code& ec)
    {
        c.disconnect();
        hasDone = true;
        result.first = false;
        result.second = boost::system::system_error(ec).what();
        blocker.notify();
    });


    GetRequest request;
    request.Type = resType;
    request.TradingDay = tradingDay;
    request.UserName = userName;
    request.Password = password;
    auto pMessage = TSerializer::serialize(g_get_constants.WMESSAGE_V1_GET,0,request);
    mConnection->send(pMessage);
    blocker.wait([&hasDone](){return hasDone;});

    return result;
}
