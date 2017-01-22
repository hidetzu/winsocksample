// client.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//


#include <thread>
#include "communication.h"


void sendThread() {
	ConfigParam configParam;
	configParam.recvPortNum = 15001;
	configParam.sendPortNum = 15000;
	configParam.ip = "127.0.0.1";
	void* pContext = communication_clientInit(&configParam);

	while(true) {
		printf("[%s] start\n", __func__);

		RequestParam reqParam;
		reqParam.cmdType = (int32_t)CommandType::Pram1;

		reqParam.dataSize = 5;
		memcpy(reqParam.data, "HELLO", 5);

		communication_clientSend(pContext, &reqParam);
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
