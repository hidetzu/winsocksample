#pragma once

#ifdef EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

#include <cstdint>

#pragma comment(lib,"Ws2_32.lib")


enum CommandType {
	Pram1,
};

#define REQUESTDATA_MAX (1 * 1024 * 1000)

struct RequestParam {
	int32_t cmdType;
	int32_t dataSize;
	char*   data;
};

struct ResponseData {
	int32_t   bufsize;
	char*     buf;
};

struct ResponseParam {
	int32_t   cmdType;
	int32_t   result;
	ResponseData   resData;
};

struct ConfigParam {
	int sendPortNum;
	int recvPortNum;
	char* ip;
};

DLL_API typedef ResponseParam* (__stdcall *t_createResponseParam)(RequestParam* pRequestParam);

DLL_API int __stdcall communication_init(void);
DLL_API int __stdcall communication_finalize(void);

DLL_API void* __stdcall communication_serverInit(ConfigParam* pParam, t_createResponseParam createResParam);
DLL_API int __stdcall communication_serverFinalize(void* pContext);

DLL_API void* __stdcall communication_clientInit(ConfigParam* pParam);
DLL_API int __stdcall communication_clientFinalize(void* pContext);
DLL_API int __stdcall communication_clientSend(void* pContext, RequestParam* pReqParam, ResponseParam** ppResParam);