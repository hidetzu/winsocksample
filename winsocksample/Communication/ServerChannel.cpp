
#include "communication.h"
#include "common_private.h"
#include "ServerChannel.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

namespace Communication {
	ServerChannel::ServerChannel(int recvPortNum, int sendPortNum, const std::string ip, t_createResponseParam createResParam) {

		this->createResParam_ = createResParam;
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

	void ServerChannel::responseHandler(RequestParam* pRequestParam) {
		auto callback = this->createResParam_;
		ResponseParam* pResponseData = nullptr;

		if (callback != nullptr) {
			pResponseData = this->createResParam_(pRequestParam);
		}

		delete[] pRequestParam->data;
		delete[] pRequestParam;

		resQueue->enqueue(pResponseData);
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
			RequestParam* pRequestParam = new RequestParam;
			memset(pRequestParam, 0x00, sizeof(RequestParam));

			int n = recv(this->recvSoc, (char*)pRequestParam, sizeof(int32_t) + sizeof(int32_t), 0);
			DEBUG_PRINT("recv <<<");
			DEBUG_PRINT("cmdType[%d] ", pRequestParam->cmdType);
			DEBUG_PRINT("dataSize[%d] ", pRequestParam->dataSize);

			pRequestParam->data = new char[pRequestParam->dataSize];
			memset(pRequestParam->data, 0x00, pRequestParam->dataSize);

			n = recv(this->recvSoc, pRequestParam->data, pRequestParam->dataSize, 0);
			DEBUG_PRINT("recv recvSize[%d]", n);
			if (n <= 0)
				break;

			std::thread a = std::thread([&] { responseHandler(pRequestParam); });
			a.detach();
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