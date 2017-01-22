
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

void* __stdcall communication_serverInit(void)
{
	return new Communication::ServerChannel(15000, 15001, "127.0.0.1");
}

int __stdcall communication_serverFinalize(void* pContext)
{
	Communication::ServerChannel* inst =
		(Communication::ServerChannel*)pContext;
	delete inst;
	return 0;
}

void* __stdcall communication_clientInit(void)
{
	return new Communication::ClientChannel(15000, 15001, "127.0.0.1");
}

int __stdcall communication_clientSend(void* pContext, char* pData, int dataSize) {
	Communication::ClientChannel* inst =
		(Communication::ClientChannel*)pContext;
	RequestParam reqParam;

	reqParam.cmdType  = (int32_t)CommandType::Pram1;
	reqParam.dataSize = dataSize + sizeof(int32_t) + sizeof(int32_t);
	memcpy(reqParam.data, pData, dataSize);

	return inst->Send(&reqParam);
}

int __stdcall communication_clientFinalize(void* pContext)
{
	Communication::ClientChannel* inst =
		(Communication::ClientChannel*)pContext;
	delete inst;
	return 0;
}
