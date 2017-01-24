
#include "communication.h"
#include "common_private.h"
#include "ClientChannel.h"
#include "util.h"

#include <assert.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/file_sinks.h>

using spdlog::sinks::stdout_sink_mt;
using spdlog::sinks::simple_file_sink_mt;
using spdlog::sinks::rotating_file_sink_mt;
using spdlog::create;

namespace Communication {

	ClientChannel::ClientChannel(int sendPortNum, int recvPortNum, char* ip) {
		this->sendPortNum = sendPortNum;
		this->recvPortNum = recvPortNum;
		this->ip = ip;
		this->cond_val = -1;

		this->resQueue = new SafeQueue< ResponseParam* >();

		auto my_logger = spdlog::basic_logger_mt("basic_logger", "basic.txt");
		spdlog::set_level(spdlog::level::info);
		my_logger->info("Some log message");
		my_logger->info("Some log message2");
		my_logger->info("Some log message3");
		my_logger->info("Some log message4");
		my_logger->info("Some log message5");
		my_logger->info("Some log message6");
		my_logger->info("Some log message7");

		recvThread = std::thread([this] { recvThreadProc(); });

		// 受信側のコネクト待ちをまつ。
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 0 == cond_val; });
		}

		this->sendSoc = 
			createSendSocket(this->sendPortNum,this->ip);

		// 受信側が通信できるようになるまで待つ
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 1 == cond_val; });
		}

		my_logger->debug("sendSoc[%d]", this->sendSoc);
		my_logger->debug("End");
	}

	ClientChannel::~ClientChannel() {
		if (this->resQueue != nullptr)
			delete this->resQueue;
	}

	int ClientChannel::Send(RequestParam* pRequestParam) {
		int32_t sendSize = pRequestParam->dataSize + sizeof(int32_t) + sizeof(int32_t);
		DEBUG_PRINT("sendSize[%d]", sendSize);
		DEBUG_PRINT("cmdType[%d]",  pRequestParam->cmdType);
		DEBUG_PRINT("bufSize[%d]",  pRequestParam->dataSize);
		assert(pRequestParam->dataSize != 0);
		return send(this->sendSoc, (const char*)pRequestParam, sendSize, 0);
	}

	ResponseParam* ClientChannel::Recv(void) {
		auto response = this->resQueue->dequeue();
		DEBUG_PRINT("recive end <<<");
		DEBUG_PRINT("END");

		return response;
	}


	void ClientChannel::recvThreadProc() {
		DEBUG_PRINT("Start");

		std::this_thread::sleep_for(std::chrono::seconds(5));

		auto listenSoc = createServerSocket(this->recvPortNum);

		// 受信側のコネクト待ちを通知する
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cond_val = 0;
		}
		cond_.notify_one();

		this->recvSoc = createAcceptSocket(listenSoc);

		// 受信側の準備が完了したことを通知する
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cond_val = 1;
		}
		cond_.notify_one();

		while (true) {
			// 受信データがそろったらクライアントへ通知する
			DEBUG_PRINT("recive loop start >>>>");

			ResponseParam* pResponseData = new ResponseParam;
			memset(pResponseData, 0, sizeof(ResponseParam));
			DEBUG_PRINT("recive start >>>>");

			int n = recv(this->recvSoc, (char*)(pResponseData),
				sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t), 0);

			ResponseData* pResData = &pResponseData->resData;

			DEBUG_PRINT("recive : size[%d] commandType[%d] result[%d] bufsize[%d]", n,
				(pResponseData->cmdType),
				(pResponseData->result),
				(pResData->bufsize));

			pResData->buf = new char[pResData->bufsize];

			recv_allData(this->recvSoc,
				pResponseData->resData.buf, pResData->bufsize);

			this->resQueue->enqueue(pResponseData);

			DEBUG_PRINT("recive loop end <<<");
		}

	}

}