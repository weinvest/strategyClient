#ifndef STRATEGYCLIENT_H
#define STRATEGYCLIENT_H
#include <boost/asio.hpp>
#include "strategyclient_global.h"
#include "common/WTraits.h"
#include "proto/common_types.h"
class TcpClient;
namespace boost
{
class thread;
}

class STRATEGYCLIENTSHARED_EXPORT StrategyClient
{

public:
    StrategyClient();

    typedef std::pair<bool,std::string> Result;
    Result connect(std::string& host,int port);

    Result  getResource(ResType::type resType
                        ,const std::string& userName
                        ,const std::string& password
                        ,const std::string& tradingDay);

private:
    Pointer<boost::asio::io_service>::Ptr mIoService;
    Pointer<boost::asio::io_service::work>::Ptr mWork;
    Pointer<TcpClient>::Ptr mConnection;
    Pointer<boost::thread>::Ptr mWorkThread;
};

#endif // STRATEGYCLIENT_H
