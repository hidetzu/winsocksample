#pragma once

#include "common_private.h"

namespace Communication {
	SOCKET createSendSocket(u_short portNum, const std::string ip);
	
	SOCKET createServerSocket(u_short portNum);
	SOCKET createAcceptSocket(SOCKET listenSoc);
	int recv_allData(SOCKET soc, char* pBuf, int32_t bufSize);

	std::string createDispTimestamp(std::time_t* pTm);
};
