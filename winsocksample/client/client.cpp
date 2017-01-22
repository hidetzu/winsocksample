// client.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h> 

#include <ctime>
#include <time.h>

#include <iostream>
#include <thread>
#include "communication.h"

#include <fstream>


void sendThread() {
	ConfigParam configParam;
	configParam.recvPortNum = 15001;
	configParam.sendPortNum = 15000;
	configParam.ip = "127.0.0.1";
	void* pContext = communication_clientInit(&configParam);

	while(true) {
		printf("[%s] start\n", __func__);

		RequestParam reqParam;
		ResponseParam* pResParam;
		reqParam.cmdType = (int32_t)CommandType::Pram1;

		reqParam.dataSize = 5;
		reqParam.data = new char[5];
		memcpy(reqParam.data, "HELLO", 5);

		communication_clientSendRecv(pContext, &reqParam, &pResParam);

		std::time_t now = std::time(NULL);
		std::tm tm;
		localtime_s(&tm, &now);

		char buffer[32];
		std::strftime(buffer, 32, "%Y%m%d%H%M%S", &tm);


		// 受信内容を取り出す
		std::ofstream  fout;
		fout.open("./recv_" + std::string(buffer) + ".jpg", std::ios::out | std::ios::binary | std::ios::trunc);

		auto pResData = pResParam;
		fout.write(reinterpret_cast<char *>(pResData->resData.buf), pResData->resData.bufsize);
		delete[] pResData->resData.buf;
		delete   pResData;


		printf("[%s] recive end <<<\n", __func__);
		delete[] reqParam.data;
	}

	communication_clientFinalize(pContext);
	printf("good by!\n");
}


int main()
{
	_CrtDumpMemoryLeaks();

	communication_init();

	std::thread th(sendThread);
    th.join();

	communication_finalize();
    return 0;
}
