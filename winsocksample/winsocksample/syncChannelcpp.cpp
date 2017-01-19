
#include "stdafx.h"
#include "syncChannel.h"

SyncChannel::SyncChannel() {
}

SyncChannel::~SyncChannel() {
}

void SyncChannel::stbOn() {
	this->set_sig(SIG_STB);
}

void SyncChannel::waitStbOn() {
	this->wait_sigActive(SIG_STB);
}

void SyncChannel::ackOn() {
	this->set_sig(SIG_ACK);
}

void SyncChannel::waitAckOn() {
	this->wait_sigActive(SIG_ACK);
}

void SyncChannel::set_sig(SigType sig) {
	{
		std::lock_guard<std::mutex> lock(_mtx);
		sig_flag = sig;
	}
	_cond.notify_one();
}

void SyncChannel::wait_sigActive(SigType sig) {
	{
		std::unique_lock<std::mutex> uniq_lk(_mtx);
		_cond.wait(uniq_lk, [&] { return this->sig_flag == sig; });
	}
}

extern "C" __declspec(dllexport) SyncChannel* createClass(void)
{
	return new SyncChannel();
}