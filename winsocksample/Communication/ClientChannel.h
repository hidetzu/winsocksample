#pragma once

#include <WinSock2.h>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "common_private.h"
#include "SafeQueue.h"

namespace Communication {
	class ClientChannel{

	public:
		ClientChannel(int sendPortNum, int recvPortNum, const std::string ip);

		int Send(RequestParam* pRequestParam);

		~ClientChannel();
	private:
		int sendPortNum;
		int recvPortNum;

		std::string ip;

		SOCKET sendSoc;
		SOCKET recvSoc;

		std::mutex mutex_;
		std::condition_variable cond_;
		int cond_val;

		std::mutex recvMutex_;
		std::condition_variable recvCond_;
		std::vector<ResponseParam*> response;
		int recvCond_val;

		SafeQueue< std::vector<ResponseParam*> >* resQueue;

		std::thread recvThread;

		void recvThreadProc();
	};
}