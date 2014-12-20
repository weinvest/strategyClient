#include <boost/asio.hpp>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include "common/WTraits.h"
#include "StrategyClient.h"
#include "network/TcpClient.h"
#include <boost/thread.hpp>
#include "common/WBlocker.h"
#include "proto/get_constants.h"
#include "proto/get_types.h"
#include "network/TSerializer.h"
#include "crypto/Crypto.h"
#include "proto/common_types.h"

class StrategyClientImpl
{

public:
	StrategyClientImpl();
	~StrategyClientImpl();

	typedef std::pair<bool, std::string> Result;
	Result connect(std::string& host, int port);

	Result  getResource(RSType::type resType
		, const std::string& userName
		, const std::string& password
		, const std::string& tradingDay);

private:
	Pointer<boost::asio::io_service>::Ptr mIoService;
	Pointer<boost::asio::io_service::work>::Ptr mWork;
	Pointer<TcpClient>::Ptr mConnection;
	Pointer<boost::thread>::Ptr mWorkThread;
};

StrategyClientImpl::StrategyClientImpl()
    :mIoService(new boost::asio::io_service())
    ,mWork(new boost::asio::io_service::work(*mIoService))
    ,mConnection(new TcpClient(*mIoService))
    ,mWorkThread(new boost::thread(boost::bind(&boost::asio::io_service::run,mIoService)))
{
}

StrategyClientImpl::~StrategyClientImpl()
{
}

StrategyClientImpl::Result StrategyClientImpl::connect(std::string& host,int port)
{
    StrategyClientImpl::Result result;
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

StrategyClientImpl::Result StrategyClientImpl::getResource(RSType::type resType
                                                   ,const std::string& userName
                                                   ,const std::string& password
                                                   ,const std::string& tradingDay)
{
    StrategyClientImpl::Result result;
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
            if(ResType::Strategy == response.Type)
            {
                using namespace boost::iostreams;
                std::stringstream sContent(response.Content),oStream;
                filtering_streambuf<input> in;
                in.push(bzip2_decompressor());
                in.push(sContent);
                boost::iostreams::copy(in,oStream);
                response.Content = oStream.str();
            }
            result.second.resize(response.Content.size());
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
    request.Type = (ResType::type)resType;
    request.TradingDay = tradingDay;
    request.UserName = userName;
    request.Password = password;
    auto pMessage = TSerializer::serialize(g_get_constants.WMESSAGE_V1_GET,0,request);
    mConnection->send(pMessage);
    blocker.wait([&hasDone](){return hasDone;});

    return result;
}

StrategyClient::StrategyClient()
:mImpl(new StrategyClientImpl())
{}


StrategyClient::~StrategyClient()
{
	delete mImpl;
}

StrategyClient::Result StrategyClient::connect(std::string& host, int port)
{
	return mImpl->connect(host, port);
}

StrategyClient::Result  StrategyClient::getResource(RSType::type resType
	, const std::string& userName
	, const std::string& password
	, const std::string& tradingDay)
{
	return mImpl->getResource(resType, userName, password, tradingDay);
}
