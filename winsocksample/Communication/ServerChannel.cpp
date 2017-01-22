
#include "communication.h"
#include "common_private.h"
#include "ServerChannel.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

//DEBUG用
#include <fstream>

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

		this->recvSoc = createAcceptSocket(this->recvPortNum);

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
			memset(buf, 0, 32);

			DEBUG_PRINT("recv >>>");
			RequestParam requestParam;
			memset(&requestParam, 0x00, sizeof(RequestParam));

			int n = recv(this->recvSoc, (char*)&requestParam, sizeof(int32_t) + sizeof(int32_t), 0);
			DEBUG_PRINT("recv <<<");
			DEBUG_PRINT("cmdType[%d] ", requestParam.cmdType);
			DEBUG_PRINT("dataSize[%d] ", requestParam.dataSize);

			n = recv(this->recvSoc, requestParam.data, requestParam.dataSize - sizeof(int32_t) - sizeof(int32_t), 0);
			DEBUG_PRINT("recv recvSize[%d]", n);
			if (n <= 0)
				break;

			{
				std::lock_guard<std::mutex> lock(sendMutex_);

				std::ifstream fin("./dambo3.jpg", std::ios::in | std::ios::binary);
				size_t fileSize = (size_t)fin.seekg(0, std::ios::end).tellg();
				fin.seekg(0, std::ios::beg);

				ResponseParam* pResponseData = new ResponseParam;
				pResponseData->cmdType = 0;
				pResponseData->result = 0;
				pResponseData->resData.continueFlg = 0;
				pResponseData->resData.buf = new char[fileSize];
				pResponseData->resData.bufsize = (int32_t)fileSize;
				fin.read(pResponseData->resData.buf, fileSize);

				this->response.push_back(pResponseData);

				sendCond_val = 1;
			}
			sendCond_.notify_one();
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

		this->sendSoc =
			createSendSocket(this->sendPortNum, this->ip);

		// 送信側の準備が完了したことを通知する
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cond_val = 1;
		}
		cond_.notify_one();

		printf("END \n");
		while (true) {
			DEBUG_PRINT("[%s] %d, loop start >>>> \n", __func__, __LINE__);
			char* pBuf = nullptr;
			int bufsize = 0;
			// 送信側が通信できるようになるまで待つ
			{
				std::unique_lock<std::mutex> uniq_lk(sendMutex_);
				sendCond_.wait(uniq_lk, [this] { return 1 == sendCond_val; });

				DEBUG_PRINT("[%s] %d, wait <<< \n", __func__, __LINE__);
				// 空になるまで送信する
				std::vector<ResponseParam*>::iterator it = this->response.begin();
				while (it != this->response.end()) {
					auto pResParam = *it;
					pResParam->cmdType = (int32_t)CommandType::Pram1;
					int n = send(this->sendSoc, (char*)(pResParam), sizeof(CommandType) + 4 + 4 + 4, 0);
					DEBUG_PRINT("recive : size[%d] continueFlg[%d] bufsize[%d]", n,
						pResParam->resData.continueFlg,
						pResParam->resData.bufsize);
					n = send(this->sendSoc, (char*)(pResParam->resData.buf), pResParam->resData.bufsize, 0);
					DEBUG_PRINT("recive : size[%d] continueFlg[%d] bufsize[%d]", n,
						pResParam->resData.continueFlg,
						pResParam->resData.bufsize);

					delete pResParam->resData.buf;
					delete pResParam;
					it = this->response.erase(it);
				}

				sendCond_val = 0;
			}

			//printf("[%s] %d, send %s >>>> \n", __func__, __LINE__, pBuf);
			
			//printf("[%s] %d, %s\n", __func__, __LINE__, pBuf);
		}
	}
}