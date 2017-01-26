
#include <stdlib.h>
#include <string.h>
#include <spdlog/spdlog.h>

#include "communication.h"
#include "common_private.h"
#include "ServerChannel.h"
#include "util.h"

namespace Communication {
	ServerChannel::ServerChannel(int recvPortNum, int sendPortNum, const std::string ip, t_createResponseParam createResParam) {

		this->createResParam_ = createResParam;
		this->recvPortNum = recvPortNum;
		this->sendPortNum = sendPortNum;
		this->ip = ip;
		this->cond_val = -1;

		recvThread = std::thread([this] { recvThreadProc(); });
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 0 == cond_val; });
		}
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

		std::this_thread::sleep_for(std::chrono::seconds(5));

		if (callback != nullptr) {
			pResponseData = this->createResParam_(pRequestParam);
		}

		my_logger->debug("cmdType[{}]", pRequestParam->cmdType);

		delete[] pRequestParam->data;
		delete   pRequestParam;

		auto pResParam = pResponseData;
		{
			int n = send(this->sendSoc, (char*)(pResParam),
				Command_GetResponseParamHeaderSize(), 0);
			my_logger->debug("send : size[{}] result[{}] bufsize[{}]", n,
				pResParam->result,
				pResParam->resData.bufsize);
			n = send(this->sendSoc, (char*)(pResParam->resData.buf), pResParam->resData.bufsize, 0);
			my_logger->debug("send : size[{}]  bufsize[{}]", n,
				pResParam->resData.bufsize);

			delete[] pResParam->resData.buf;
			delete pResParam;
		}
		my_logger->debug("Send Loop End <<< ");
	}


	void ServerChannel::recvThreadProc() {
		auto soc = createServerSocket(this->recvPortNum);
		this->recvSoc = createAcceptSocket(soc);
		this->sendSoc =
			createSendSocket(this->sendPortNum, this->ip);

		// óM‘¤‚Ì€”õ‚ªŠ®—¹‚µ‚½‚±‚Æ‚ğ’Ê’m‚·‚é
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cond_val = 0;
		}
		cond_.notify_one();

		while (true) {
			RequestParam* pRequestParam = new RequestParam;
			memset(pRequestParam, 0x00, sizeof(RequestParam));

			int n = recv(this->recvSoc, (char*)pRequestParam, Command_GetRequestParamHeaderSize(), 0);
			my_logger->debug("recv [{}]<<<", n);
			my_logger->debug("cmdType[{}] ", pRequestParam->cmdType);
			my_logger->debug("dataSize[{}] ", pRequestParam->dataSize);

			pRequestParam->data = new char[pRequestParam->dataSize];
			memset(pRequestParam->data, 0x00, pRequestParam->dataSize);

			n = recv(this->recvSoc, pRequestParam->data, pRequestParam->dataSize, 0);
			my_logger->debug("recv recvSize[{}]", n);
			if (n <= 0)
				break;

#if true
			std::thread a = std::thread([&, pRequestParam] { responseHandler(pRequestParam); });
			a.detach();
#else
			responseHandler(pRequestParam);
#endif
		}
	}

}