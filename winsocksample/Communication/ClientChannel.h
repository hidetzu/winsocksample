#pragma once

#include <WinSock2.h>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace Communication {
	class ClientChannel{

	public:
		ClientChannel(int sendPortNum, int recvPortNum, const std::string ip);

		int Send(char* pData, int dataSize);

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
		char* recvBuf;
		int recvBufSize;
		int recvCond_val;

		std::thread recvThread;

		void recvThreadProc();
	};
}