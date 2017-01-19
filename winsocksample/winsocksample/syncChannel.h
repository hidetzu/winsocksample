#pragma once

#include <mutex>
#include <condition_variable>

class SyncChannel
{
public:
	SyncChannel();
	~SyncChannel();

	void stbOn();
	void waitStbOn();
	void ackOn();
	void waitAckOn();

private:
	typedef enum {
		SIG_STB = 1 << 0,
		SIG_ACK = 1 << 1,

		SIG_MASK = SIG_STB |
		SIG_ACK,
	}SigType;

	std::mutex _mtx;
	std::condition_variable _cond;
	SigType sig_flag;

	void set_sig(SigType sig);
	 void wait_sigActive(SigType sig);
};

extern "C" __declspec(dllexport) SyncChannel* createClass(void);
