
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
		this->cond_val = -1;

		this->resQueue = new SafeQueue< std::vector<ResponseParam*> >();

		recvThread = std::thread([this] { recvThreadProc(); });
		sendThread = std::thread([this] { sendThreadProc(); });
	}

	ServerChannel::~ServerChannel() {
		if (recvThread.joinable())
			recvThread.join();

		if (this->resQueue != nullptr) {
			delete this->resQueue;
		}
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

			std::vector<ResponseParam*> response;
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

			response.push_back(pResponseData);

			resQueue->enqueue(response);

#if false
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
#endif

		}
	}

	void ServerChannel::sendThreadProc() {

		DEBUG_PRINT("pripare recv Connection  ...");
		// 受信側が通信できるようになるまで待つ
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 0 == cond_val; });
		}
		std::this_thread::sleep_for(std::chrono::seconds(2));

		DEBUG_PRINT("Start");

		this->sendSoc =
			createSendSocket(this->sendPortNum, this->ip);

		// 送信側の準備が完了したことを通知する
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cond_val = 1;
		}
		cond_.notify_one();

		DEBUG_PRINT("End");

		while (true) {
			DEBUG_PRINT("Send Loop Start >>>");

			// 送信データの準備ができるまで待つ。
			std::vector<ResponseParam*> response =
				this->resQueue->dequeue();
			{
				DEBUG_PRINT("recv sendResponse");
				// 空になるまで送信する
				std::vector<ResponseParam*>::iterator it = response.begin();
				while (it != response.end()) {
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
					it = response.erase(it);
				}
			}
			DEBUG_PRINT("Send Loop End <<< ");
		}
	}
}