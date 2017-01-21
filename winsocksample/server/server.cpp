// winsocksample.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include <stdio.h>
#include "communication.h"

int main() {
	communication_init();

	void* pContext = communication_serverInit();
	communication_serverFinalize(pContext);

	communication_finalize();

	getwchar();
	return 0;
}