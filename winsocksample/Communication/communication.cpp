
#include <WinSock2.h>

#include "communication.h"
#include "ServerChannel.h"
#include "ClientChannel.h"

int __stdcall communication_init(void)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	return 0;
}

int __stdcall communication_finalize(void)
{
	WSACleanup();
	return 0;
}

void* __stdcall communication_serverInit(ConfigParam* pParam, t_createResponseParam createResParam)
{
	return new Communication::ServerChannel(pParam->recvPortNum, pParam->sendPortNum, pParam->ip, createResParam);
}

int __stdcall communication_serverFinalize(void* pContext)
{
	Communication::ServerChannel* inst =
		(Communication::ServerChannel*)pContext;
	delete inst;
	return 0;
}

void* __stdcall communication_clientInit(ConfigParam* pParam)
{
	return new Communication::ClientChannel(pParam->sendPortNum, pParam->recvPortNum, pParam->ip);
}

int __stdcall communication_clientSendRecv(void* pContext, RequestParam* pReqParam, ResponseParam** ppResParam) {
	Communication::ClientChannel* inst =
		(Communication::ClientChannel*)pContext;

	int ret = inst->SendRecv(pReqParam, ppResParam);

	DEBUG_PRINT("recive end <<<");
	DEBUG_PRINT("END");

	return ret;
}

int __stdcall communication_clientFinalize(void* pContext)
{
	Communication::ClientChannel* inst =
		(Communication::ClientChannel*)pContext;
	delete inst;
	return 0;
}
