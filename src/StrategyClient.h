#ifndef STRATEGYCLIENT_H
#define STRATEGYCLIENT_H

#include "strategyclient_global.h"
#include "common/WEnum.h"
//
class TcpClient;
namespace boost
{
class thread;
}

#define RSTYPE_VALUES \
	((Mapping,0))\
	((Configure,1))\
	((Strategy,3))

struct RSType
{
	enum type
	{
		WENUM_VALUE(RSTYPE_VALUES)
	};
};

class STRATEGYCLIENTSHARED_EXPORT StrategyClient
{

public:
    StrategyClient();
    ~StrategyClient();

    typedef std::pair<bool,std::string> Result;
    Result connect(std::string& host,int port);

    Result  getResource(RSType::type resType
                        ,const std::string& userName
                        ,const std::string& password
                        ,const std::string& tradingDay);

    void close();
private:
	class StrategyClientImpl* mImpl;
};

#endif // STRATEGYCLIENT_H
