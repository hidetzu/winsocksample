// client.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>

#include "communication.h"

std::mutex _mtx;
std::condition_variable _cond;
bool _isReady = false;

void recvThread() {
	printf("[%s];%d: start\n", __func__, __LINE__);
	SOCKET sock;
	BOOL yes = 1;
	char buf[32];

	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len;

	// �\�P�b�g�̍쐬
	sock = socket(AF_INET, SOCK_STREAM, 0);

	// �ڑ���w��p�\���̂̏���
	addr.sin_family = AF_INET;
	addr.sin_port = htons(15001);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(sock, (struct sockaddr *)&addr, sizeof(addr));

	// TCP�N���C�A���g����̐ڑ��v����҂Ă��Ԃɂ���
	listen(sock, 5);
	printf("[%s];%d: start\n", __func__, __LINE__);

	// TCP�N���C�A���g����̐ڑ��v�����󂯕t����
	len = sizeof(client);
	sock = accept(sock, (struct sockaddr *)&client, &len);
	printf("[%s];%d: start\n", __func__, __LINE__);

	while (true) {
		printf("[%s] start\n", __func__);
		// �N���C�A���g����f�[�^����M
		memset(buf, 0, sizeof(buf));
		int n = recv(sock, buf, 5, 0);
		if (n <= 0)
			break;

		{
			std::lock_guard<std::mutex> lock(_mtx);
			_isReady = true;
		}
		_cond.notify_one();
		{
			std::unique_lock<std::mutex> uniq_lk(_mtx); // �����Ń��b�N�����
			_cond.wait(uniq_lk, [] { return !_isReady; });
		}
		printf("%d, %s\n", n, buf);
		printf("[%s] end\n", __func__);
	}
}

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

//	std::thread th2(recvThread);
	std::thread th(sendThread);

    th.join();
//	th2.join();

	communication_finalize();
    return 0;
}

