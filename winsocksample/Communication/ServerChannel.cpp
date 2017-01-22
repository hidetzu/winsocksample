
#include "communication.h"
#include "common_private.h"
#include "ServerChannel.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

//DEBUG用
#include <fstream>

namespace Communication {
	ServerChannel::ServerChannel(int recvPortNum, int sendPortNum, std::string ip) {
		this->recvPortNum = recvPortNum;
		this->sendPortNum = sendPortNum;
		this->ip = ip;
		this->cond_val = -1;

		this->resQueue = new SafeQueue< ResponseParam* >();

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

			n = recv(this->recvSoc, requestParam.data, requestParam.dataSize, 0);
			DEBUG_PRINT("recv recvSize[%d]", n);
			if (n <= 0)
				break;

			std::ifstream fin("./dambo3.jpg", std::ios::in | std::ios::binary);
			size_t fileSize = (size_t)fin.seekg(0, std::ios::end).tellg();
			fin.seekg(0, std::ios::beg);

			ResponseParam* pResponseData = new ResponseParam;
			pResponseData->cmdType = 0;
			pResponseData->result = 0;
			pResponseData->resData.buf = new char[fileSize];
			pResponseData->resData.bufsize = (int32_t)fileSize;
			fin.read(pResponseData->resData.buf, fileSize);

			resQueue->enqueue(pResponseData);
		}
	}

	void ServerChannel::sendThreadProc() {

		DEBUG_PRINT("pripare recv Connection  ...");
		// 受信側が通信できるようになるまで待つ
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 0 == cond_val; });
		}

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
			auto pResParam = this->resQueue->dequeue();
			{
				DEBUG_PRINT("recv sendResponse");

				int n = send(this->sendSoc, (char*)(pResParam), sizeof(CommandType) + 4 + 4, 0);
				DEBUG_PRINT("recive : size[%d] result[%d] bufsize[%d]", n,
					pResParam->result,
					pResParam->resData.bufsize);
				n = send(this->sendSoc, (char*)(pResParam->resData.buf), pResParam->resData.bufsize, 0);
				DEBUG_PRINT("recive : size[%d]  bufsize[%d]", n,
					pResParam->resData.bufsize);

				delete pResParam->resData.buf;
				delete pResParam;
			}
			DEBUG_PRINT("Send Loop End <<< ");
		}
	}
}