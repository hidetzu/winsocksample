#pragma once

#include <WinSock2.h>
#include <spdlog/spdlog.h>

#include "common_private.h"

namespace Communication {
	class ClientChannel{

	public:
		ClientChannel(int sendPortNum, int recvPortNum,  char* ip);

		int SendRecv(RequestParam* pRequestParam, ResponseParam** ppResParam);

		~ClientChannel();
	private:
		int sendPortNum;
		int recvPortNum;

		std::string ip;

		SOCKET sendSoc;
		SOCKET recvSoc;

		std::shared_ptr<spdlog::logger> my_logger;
	};
}