
#include "communication.h"
#include "common_private.h"
#include "ClientChannel.h"
#include "util.h"

#include <fstream>

namespace Communication {

	ClientChannel::ClientChannel(int sendPortNum, int recvPortNum, const std::string ip) {
		this->sendPortNum = sendPortNum;
		this->recvPortNum = recvPortNum;
		this->ip = ip;
		this->cond_val = -1;

		recvThread = std::thread([this] { recvThreadProc(); });

		this->sendSoc = 
			createSendSocket(this->sendPortNum,this->ip);

		this->resQueue = new SafeQueue< std::vector<ResponseParam*> >();

		// ��M�����ʐM�ł���悤�ɂȂ�܂ő҂�
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 0 == cond_val; });
		}

		DEBUG_PRINT("END");
	}

	ClientChannel::~ClientChannel() {
		if (this->resQueue != nullptr)
			delete this->resQueue;
	}

	int ClientChannel::Send(RequestParam* pRequestParam) {
		send(this->sendSoc, (const char*)pRequestParam, pRequestParam->dataSize, 0);
		DEBUG_PRINT("wait recive >>>>");

		std::vector<ResponseParam*> response = this->resQueue->dequeue();

		// ��M���e�����o��
		std::ofstream  fout;
		fout.open("./recv.jpg", std::ios::out | std::ios::binary | std::ios::trunc);

		std::vector<ResponseParam*>::iterator it = response.begin();
		while (it != response.end()) {
			auto pResData = *it;
			fout.write(reinterpret_cast<char *>(pResData->resData.buf), pResData->resData.bufsize);

			delete pResData->resData.buf;
			delete pResData;
			it = response.erase(it);
		}
		DEBUG_PRINT("recive end <<<");
		DEBUG_PRINT("END");

		return 0;
	}

	void ClientChannel::recvThreadProc() {
		DEBUG_PRINT("Start");

		this->recvSoc = createAcceptSocket(this->recvPortNum);

		// ��M���̏����������������Ƃ�ʒm����
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cond_val = 0;
		}
		cond_.notify_one();

		while (true) {
			// ��M�f�[�^�����������N���C�A���g�֒ʒm����
			DEBUG_PRINT("recive loop start >>>>");

			std::vector<ResponseParam*> response;
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

			response.push_back(pResponseData);
			this->resQueue->enqueue(response);

			DEBUG_PRINT("recive loop end <<<");
		}

	}

}