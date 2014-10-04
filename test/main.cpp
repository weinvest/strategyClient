#include <iostream>
#include "src/StrategyClient.h"
#include "proto/get_types.h"
#include "proto/common_types.h"
using namespace std;

int main()
{
    std::string host("106.186.123.105");
    int port = 5188;
//    std::cout<<"Host:";
//    std::cin >> host;
//    std::cout<<"Port:";
//    std::cin>>port;

    StrategyClient client;
    auto result = client.connect(host,port);
    if(result.first)
    {
        std::cout<<"successed connect to "<<host<<":"<<port<<std::endl;
    }
    else
    {
        std::cout<<"connect failed:" << result.second<<std::endl;
        return 0;
    }

    std::string line;
    while(std::getline(std::cin,line))
    {
        int resType;
        std::string userName;
        std::string password;
        std::string tradingDay;

        resType = 2;
        userName = "g@dFM1Vs";
        password = "q%4d1FzePiu8";
        tradingDay = "xx";
//        std::cout<<"ResType(0-mapping 1-configure 2-strategy):";
//        std::cin>>resType;
//        std::cout<<"UserName:";
//        std::cin>>userName;
//        std::cout<<"Password:";
//        std::cin>>password;
//        std::cout<<"TradingDay:";
//        std::cin>>tradingDay;

        auto result = client.getResource(RSType::type(resType),userName,password,tradingDay);
        if(result.first)
        {
            std::cout<<"successed,content:"<<std::endl;
            std::cout<<result.second<<std::endl;
        }
        else
        {
            std::cout<<"failed,reason: " << result.second<<std::endl;
        }
    }

    return 0;
}

