
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

		// ソケットの作成
		sock = socket(AF_INET, SOCK_STREAM, 0);

		// 接続先指定用構造体の準備
		addr.sin_family = AF_INET;
		addr.sin_port = htons(this->recvPortNum);
		addr.sin_addr.S_un.S_addr = INADDR_ANY;

		bind(sock, (struct sockaddr *)&addr, sizeof(addr));

		// TCPクライアントからの接続要求を待てる状態にする
		listen(sock, 5);

		// TCPクライアントからの接続要求を受け付ける
		len = sizeof(client);
		this->recvSoc = accept(sock, (struct sockaddr *)&client, &len);

		// 受信側の準備が完了したことを通知する
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cond_val = 0;
		}
		cond_.notify_one();

		// 送信側が通信できるようになるまで待つ
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 1 == cond_val; });
		}

		while (true) {
			char buf[32];

			// クライアントからデータを受信
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

		// 受信側が通信できるようになるまで待つ
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 0 == cond_val; });
		}
		std::this_thread::sleep_for(std::chrono::seconds(2));
		printf("[%s];%d: sendThreadProc start %d\n", __func__, __LINE__, sendPortNum);

		// ソケットの作成
		this->sendSoc = socket(AF_INET, SOCK_STREAM, 0);

		// 接続先指定用構造体の準備
		server.sin_family = AF_INET;
		server.sin_port = htons(this->sendPortNum);
		inet_pton(AF_INET, (PCSTR)this->ip.c_str(), &server.sin_addr);

		// サーバに接続
		int ret = connect(this->sendSoc, (struct sockaddr *)&server, sizeof(server));
		printf("[%s];%d: ret[%d]\n", __func__, __LINE__, ret);

		// 送信側の準備が完了したことを通知する
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
			// 送信側が通信できるようになるまで待つ
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
			// 5文字送信
			int n = send(this->sendSoc, pBuf, bufsize, 0);
			printf("[%s] %d, %s\n", __func__, __LINE__, pBuf);
		}
	}
}