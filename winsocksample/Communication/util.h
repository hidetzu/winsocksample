#pragma once

#include "common_private.h"

namespace Communication {
	SOCKET createSendSocket(u_short portNum, const std::string ip);
	
	SOCKET createAcceptSocket(u_short portNum);
	int recv_allData(SOCKET soc, char* pBuf, int32_t bufSize);
};
