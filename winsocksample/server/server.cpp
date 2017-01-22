// winsocksample.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include <stdio.h>
#include "communication.h"

int main() {
	communication_init();

	void* pContext = communication_serverInit(15000, 15001, "127.0.0.1");
	communication_serverFinalize(pContext);

	communication_finalize();

	getwchar();
	return 0;
}