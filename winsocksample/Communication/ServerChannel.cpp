
#include "common_private.h"
#include "ServerChannel.h"
#include <stdlib.h>
#include <string.h>

namespace Communication {
	ServerChannel::ServerChannel(int recvPortNum, int sendPortNum, const std::string ip) {
		this->recvPortNum = recvPortNum;
		this->sendPortNum = sendPortNum;
		this->ip = ip;
		this->sendCond_val = 0;
		this->cond_val = -1;
		recvThread = std::thread([this] { recvThreadProc(); });
		sendThread = std::thread([this] { sendThreadProc(); });
	}

	ServerChannel::~ServerChannel() {
		if (recvThread.joinable())
			recvThread.join();
	}

	void ServerChannel::recvThreadProc() {
		SOCKET sock;

		struct sockaddr_in addr;
		struct sockaddr_in client;
		int len;

		// �\�P�b�g�̍쐬
		sock = socket(AF_INET, SOCK_STREAM, 0);

		// �ڑ���w��p�\���̂̏���
		addr.sin_family = AF_INET;
		addr.sin_port = htons(this->recvPortNum);
		addr.sin_addr.S_un.S_addr = INADDR_ANY;

		bind(sock, (struct sockaddr *)&addr, sizeof(addr));

		// TCP�N���C�A���g����̐ڑ��v����҂Ă��Ԃɂ���
		listen(sock, 5);

		// TCP�N���C�A���g����̐ڑ��v�����󂯕t����
		len = sizeof(client);
		this->recvSoc = accept(sock, (struct sockaddr *)&client, &len);

		// ��M���̏����������������Ƃ�ʒm����
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cond_val = 0;
		}
		cond_.notify_one();

		// ���M�����ʐM�ł���悤�ɂȂ�܂ő҂�
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 1 == cond_val; });
		}

		while (true) {
			char buf[32];

			// �N���C�A���g����f�[�^����M
			memset(buf, 0, sizeof(buf));
			printf("[%s] %d, recv >>>> \n", __func__, __LINE__);
			int n = recv(this->recvSoc, buf, 5, 0);
			printf("[%s] %d, recv <<< \n", __func__, __LINE__);
			if (n <= 0)
				break;

			{
				std::lock_guard<std::mutex> lock(sendMutex_);
				sendBufSize = n;
				sendBuf = (char*)malloc(sizeof(char) * n);
				memset(sendBuf, '0x00', sizeof(char) * n);
				memcpy(sendBuf, buf, sizeof(char) * n);
				sendCond_val = 1;
			}
			sendCond_.notify_one();

			printf("[%s] %d, %s %d\n", __func__, n, buf, sizeof(sendBuf));
		}
	}

	void ServerChannel::sendThreadProc() {
		struct sockaddr_in server;
		//char buf[32];

		// ��M�����ʐM�ł���悤�ɂȂ�܂ő҂�
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 0 == cond_val; });
		}
		std::this_thread::sleep_for(std::chrono::seconds(2));
		printf("[%s];%d: sendThreadProc start %d\n", __func__, __LINE__, sendPortNum);

		// �\�P�b�g�̍쐬
		this->sendSoc = socket(AF_INET, SOCK_STREAM, 0);

		// �ڑ���w��p�\���̂̏���
		server.sin_family = AF_INET;
		server.sin_port = htons(this->sendPortNum);
		inet_pton(AF_INET, (PCSTR)this->ip.c_str(), &server.sin_addr);

		// �T�[�o�ɐڑ�
		int ret = connect(this->sendSoc, (struct sockaddr *)&server, sizeof(server));
		printf("[%s];%d: ret[%d]\n", __func__, __LINE__, ret);

		// ���M���̏����������������Ƃ�ʒm����
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cond_val = 1;
		}
		cond_.notify_one();

		printf("END \n");
		while (true) {
			printf("[%s] %d, loop start >>>> \n", __func__, __LINE__);
			char* pBuf = nullptr;
			int bufsize = 0;
			// ���M�����ʐM�ł���悤�ɂȂ�܂ő҂�
			{
				std::unique_lock<std::mutex> uniq_lk(sendMutex_);
				sendCond_.wait(uniq_lk, [this] { return 1 == sendCond_val; });

				printf("[%s] %d, send %s >>>> \n", __func__, __LINE__, this->sendBuf);
				pBuf = (char*)malloc(sendBufSize);
				memset(pBuf, '\0', sendBufSize);
				memcpy(pBuf, sendBuf, sendBufSize);
				bufsize = sendBufSize;

				if(sendBuf != nullptr)
					free(sendBuf);
				sendBuf = nullptr;
				sendBufSize = 0;
				sendCond_val = 0;
			}
			printf("[%s] %d, send %s >>>> \n", __func__, __LINE__, pBuf);
			// 5�������M
			int n = send(this->sendSoc, pBuf, bufsize, 0);
			printf("[%s] %d, %s\n", __func__, __LINE__, pBuf);
		}
	}
}