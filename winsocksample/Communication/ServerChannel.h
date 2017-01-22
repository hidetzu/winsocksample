#pragma once


#include <vector> 

#include "communication.h"
#include "common_private.h"
#include "SafeQueue.h"

namespace Communication {
	class ServerChannel {
	public:
		ServerChannel(int recvPortNum, int sendPortNum, const std::string ip);
		~ServerChannel();
	private:
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

		std::mutex sendMutex_;
		std::condition_variable sendCond_;
		std::vector<ResponseParam*> response;
		int sendCond_val;

		SafeQueue< std::vector<ResponseParam*> >* resQueue;

		void recvThreadProc();
		void sendThreadProc();
	};
}
