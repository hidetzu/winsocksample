
#include "communication.h"
#include "common_private.h"
#include "ClientChannel.h"
#include "util.h"

#include <assert.h>

namespace Communication {

	ClientChannel::ClientChannel(int sendPortNum, int recvPortNum, char* ip) {
		this->sendPortNum = sendPortNum;
		this->recvPortNum = recvPortNum;
		this->ip = ip;
		this->cond_val = -1;

		this->resQueue = new SafeQueue< ResponseParam* >();

		recvThread = std::thread([this] { recvThreadProc(); });

		// ��M���̃R�l�N�g�҂����܂B
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 0 == cond_val; });
		}

		this->sendSoc = 
			createSendSocket(this->sendPortNum,this->ip);

		// ��M�����ʐM�ł���悤�ɂȂ�܂ő҂�
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 1 == cond_val; });
		}

		DEBUG_PRINT("sendSoc[%d]", this->sendSoc);

		DEBUG_PRINT("END");
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

		// ��M���̃R�l�N�g�҂���ʒm����
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cond_val = 0;
		}
		cond_.notify_one();

		this->recvSoc = createAcceptSocket(listenSoc);

		// ��M���̏����������������Ƃ�ʒm����
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cond_val = 1;
		}
		cond_.notify_one();

		while (true) {
			// ��M�f�[�^�����������N���C�A���g�֒ʒm����
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