// winsocksample.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h> 

#include <stdio.h>
#include "communication.h"

int main() {
	_CrtDumpMemoryLeaks();

	communication_init();

	ConfigParam configParam;
	configParam.recvPortNum = 15000;
	configParam.sendPortNum = 15001;
	configParam.ip          = "127.0.0.1";

	void* pContext = communication_serverInit(&configParam);
	communication_serverFinalize(pContext);

	communication_finalize();

	getwchar();
	return 0;
}