/// <summary>
/// AAAA
/// </summary>

#pragma once

#include <vector> 

#include "communication.h"
#include "common_private.h"
#include "SafeQueue.h"

#include <spdlog/spdlog.h>

namespace Communication {
	class ServerChannel {
	public:
		ServerChannel(int recvPortNum, int sendPortNum, const std::string ip, t_createResponseParam createResParam);
		~ServerChannel();
		static std::string ServerChannel::LoggerName();

	private:
		t_createResponseParam createResParam_;
		int recvPortNum;
		int sendPortNum;
		std::string ip;

		std::thread recvThread;
		SOCKET recvSoc;

		std::thread sendThread;
		SOCKET sendSoc;

		std::mutex mutex_;
		std::condition_variable cond_;
		int cond_val;

		SafeQueue< ResponseParam* >* resQueue;

		void recvThreadProc();

		void responseHandler(RequestParam* pRequestParam);


		std::shared_ptr<spdlog::logger> my_logger;
	};
}
