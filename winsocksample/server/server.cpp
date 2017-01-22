// winsocksample.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h> 

#include <stdio.h>
#include "communication.h"

#include <fstream>

static ResponseParam* __stdcall createReponse(RequestParam* pReqParam) {
	std::ifstream fin("./dambo3.jpg", std::ios::in | std::ios::binary);
	size_t fileSize = (size_t)fin.seekg(0, std::ios::end).tellg();
	fin.seekg(0, std::ios::beg);

	ResponseParam* pResponseData = new ResponseParam;
	pResponseData->cmdType = 0;
	pResponseData->result = 0;
	pResponseData->resData.buf = new char[fileSize];
	pResponseData->resData.bufsize = (int32_t)fileSize;
	fin.read(pResponseData->resData.buf, fileSize);

	return pResponseData;
}

int main() {
	_CrtDumpMemoryLeaks();

	communication_init();

	ConfigParam configParam;
	configParam.recvPortNum = 15000;
	configParam.sendPortNum = 15001;
	configParam.ip          = "127.0.0.1";

	void* pContext = communication_serverInit(&configParam, createReponse);
	communication_serverFinalize(pContext);

	communication_finalize();

	getwchar();
	return 0;
}