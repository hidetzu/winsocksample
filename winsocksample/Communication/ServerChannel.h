#pragma once

#include <WinSock2.h>
#include <thread>
#include <mutex>
#include <condition_variable>

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
		char* sendBuf;
		int sendBufSize;
		int sendCond_val;

		void recvThreadProc();
		void sendThreadProc();
	};
}
