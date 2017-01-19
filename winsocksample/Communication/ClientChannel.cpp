
#include "common_private.h"
#include "ClientChannel.h"

#include <fstream>

namespace Communication {

	ClientChannel::ClientChannel(int sendPortNum, int recvPortNum, const std::string ip) {
		this->sendPortNum = sendPortNum;
		this->recvPortNum = recvPortNum;
		this->ip = ip;
		this->cond_val = -1;

		recvThread = std::thread([this] { recvThreadProc(); });

		struct sockaddr_in server;
		char buf[32];

		// �\�P�b�g�̍쐬
		this->sendSoc = socket(AF_INET, SOCK_STREAM, 0);

		// �ڑ���w��p�\���̂̏���
		server.sin_family = AF_INET;
		server.sin_port = htons(this->sendPortNum);
		inet_pton(AF_INET, (PCSTR)this->ip.c_str(), &server.sin_addr);

		// �T�[�o�ɐڑ�
		int ret = connect(this->sendSoc, (struct sockaddr *)&server, sizeof(server));
		printf("[%s];%d: ret[%d]\n", __func__, __LINE__, ret);

		// ��M�����ʐM�ł���悤�ɂȂ�܂ő҂�
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 0 == cond_val; });
		}

		printf("END \n");
	}

	ClientChannel::~ClientChannel() {
	}

	int ClientChannel::Send(char* pData, int dataSize) {
		send(this->sendSoc, pData, dataSize, 0);

		// ��M�����ʐM�ł���悤�ɂȂ�܂ő҂�
		{
			std::unique_lock<std::mutex> uniq_lk(recvMutex_);
			recvCond_.wait(uniq_lk, [this] { return 1 == recvCond_val; });

			std::ofstream  fout;
			fout.open("./recv.jpg", std::ios::out | std::ios::binary | std::ios::trunc);
			fout.write(reinterpret_cast<char *>(recvBuf), recvBufSize);

			//printf("%s:%d %s, %d\n", __func__, __LINE__, recvBuf, recvBufSize);
			free(recvBuf);
			recvBuf = nullptr;

		}

		return 0;
	}

	void ClientChannel::recvThreadProc() {
		SOCKET sock;

		struct sockaddr_in addr;
		struct sockaddr_in client;
		int len;

		printf("%s:%d recvThreadProc Start\n", __func__, __LINE__);

		// �\�P�b�g�̍쐬
		sock = socket(AF_INET, SOCK_STREAM, 0);

		// �ڑ���w��p�\���̂̏���
		addr.sin_family = AF_INET;
		addr.sin_port = htons(this->recvPortNum);
		addr.sin_addr.S_un.S_addr = INADDR_ANY;

		bind(sock, (struct sockaddr *)&addr, sizeof(addr));

		// TCP�N���C�A���g����̐ڑ��v����҂Ă��Ԃɂ���
		listen(sock, 5);

		printf("%s:%d recvThreadProc listen %d\n", __func__, __LINE__, recvPortNum);
		// TCP�N���C�A���g����̐ڑ��v�����󂯕t����
		len = sizeof(client);
		this->recvSoc = accept(sock, (struct sockaddr *)&client, &len);

		// ��M���̏����������������Ƃ�ʒm����
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cond_val = 0;
		}
		cond_.notify_one();

		while (true) {
			char buf[17126];

			// �N���C�A���g����f�[�^����M
			memset(buf, 0, sizeof(buf));
			int n = recv(this->recvSoc, buf, 17126, 0);
			if (n <= 0)
				break;

			{
				std::lock_guard<std::mutex> lock(recvMutex_);
				recvBuf = (char*)malloc(sizeof(char) * n);
				memset(recvBuf, '0x00', sizeof(char) * n);
				memcpy(recvBuf, buf, sizeof(char) * n);
				recvBufSize = sizeof(char) * n;
				recvCond_val = 1;
			}
			recvCond_.notify_one();

			printf("[%s] %d, %s\n", __func__, n, buf);
		}

	}

}