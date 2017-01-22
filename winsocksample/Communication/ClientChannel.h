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
		ClientChannel(int sendPortNum, int recvPortNum,  char* ip);

		int Send(RequestParam* pRequestParam);
		ResponseParam* Recv(void);

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

		SafeQueue< ResponseParam* >* resQueue;

		std::thread recvThread;

		void recvThreadProc();
	};
}