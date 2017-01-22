// winsocksample.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include <stdio.h>
#include "communication.h"

int main() {
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