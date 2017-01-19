// winsocksample.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#if false
#include <winsock2.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "syncChannel.h"
#endif

#include "communication.h"

#if false
static SyncChannel* s_syncChannel = nullptr;

void recvThread() {
	SOCKET sock;
	char buf[32];

	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len;

	// �\�P�b�g�̍쐬
	sock = socket(AF_INET, SOCK_STREAM, 0);

	// �ڑ���w��p�\���̂̏���
	addr.sin_family = AF_INET;
	addr.sin_port = htons(15000);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(sock, (struct sockaddr *)&addr, sizeof(addr));


	// TCP�N���C�A���g����̐ڑ��v����҂Ă��Ԃɂ���
	listen(sock, 5);

	// TCP�N���C�A���g����̐ڑ��v�����󂯕t����
	len = sizeof(client);
	sock = accept(sock, (struct sockaddr *)&client, &len);
	s_syncChannel->ackOn();
	s_syncChannel->waitStbOn();

	while (true) {
		printf("[%s] start\n", __func__);
		// �N���C�A���g����f�[�^����M
		memset(buf, 0, sizeof(buf));
		int n = recv(sock, buf, 5, 0);
		if (n <= 0)
			break;
		printf("[%s] %d, %s\n", __func__ ,n, buf);
		s_syncChannel->ackOn();
		printf("[%s]:%d wakeup send\n", __func__, __LINE__);
		s_syncChannel->waitStbOn();
		printf("[%s]:%d wakeup recv\n", __func__, __LINE__);
		printf("[%s] end\n", __func__);
	}
}

void sendThread() {
	struct sockaddr_in server;
	SOCKET sock;
	char buf[32];

	s_syncChannel->waitAckOn();

	// �\�P�b�g�̍쐬
	sock = socket(AF_INET, SOCK_STREAM, 0);

	// �ڑ���w��p�\���̂̏���
	server.sin_family = AF_INET;
	server.sin_port = htons(15001);
	server.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	// �T�[�o�ɐڑ�
	int ret = connect(sock, (struct sockaddr *)&server, sizeof(server));
	if (ret < 0) {
		int nLastErr = WSAGetLastError();
		printf("[%s]:%d ret=%d\n", __func__, __LINE__, nLastErr);
	}
	s_syncChannel->stbOn();

	printf("[%s]:%d ret=%d\n", __func__, __LINE__, ret);

	while (true) {
		printf("[%s] send start\n", __func__);
		s_syncChannel->waitAckOn();
		printf("[%s]:%d wakeup recv\n", __func__, __LINE__);
		// 5�������M
		send(sock, "SELLO", 5, 0);
		s_syncChannel->stbOn();
		printf("[%s]:%d wakeup send\n", __func__, __LINE__);
		printf("[%s] end\n", __func__);
	}

	printf("good by!\n");
}


int main()
{
	communication_init();

	s_syncChannel = new SyncChannel();

	std::thread th(recvThread);
	std::thread th2(sendThread);
	th.join();
	th2.join();

	communication_finalize();
	s_syncChannel = nullptr;

    return 0;
}
#endif

int main() {
	communication_init();

	void* pContext = communication_serverInit();
	communication_serverFinalize(pContext);

	communication_finalize();

	getwchar();
	return 0;
}