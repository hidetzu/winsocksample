// client.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//


#include <thread>
#include "communication.h"


void sendThread() {
	void* pContext = communication_clientInit();

	while(true) {
		printf("[%s] start\n", __func__);
		communication_clientSend(pContext, "CELLO", 5);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		printf("[%s] end\n", __func__);
	}

	communication_clientFinalize(pContext);
	printf("good by!\n");
}


int main()
{
	communication_init();

	std::thread th(sendThread);
    th.join();

	communication_finalize();
    return 0;
}

