#pragma once

#ifdef EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

#pragma comment(lib,"Ws2_32.lib")

DLL_API int __stdcall communication_init(void);
DLL_API int __stdcall communication_finalize(void);

DLL_API void* __stdcall communication_serverInit(void);
DLL_API int __stdcall communication_serverFinalize(void* pContext);

DLL_API void* __stdcall communication_clientInit(void);
DLL_API int __stdcall communication_clientFinalize(void* pContext);
DLL_API int __stdcall communication_clientSend(void* pContext, char* pData, int dataSize);