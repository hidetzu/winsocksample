
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

		// 受信側が通信できるようになるまで待つ
		{
			std::unique_lock<std::mutex> uniq_lk(mutex_);
			cond_.wait(uniq_lk, [this] { return 0 == cond_val; });
		}

		DEBUG_PRINT("END");
	}

	ClientChannel::~ClientChannel() {
	}

	int ClientChannel::Send(RequestParam* pRequestParam) {
		send(this->sendSoc, (const char*)pRequestParam, pRequestParam->dataSize, 0);
		DEBUG_PRINT("wait recive >>>>");


		// 受信側が通信できるようになるまで待つ
		{
			std::unique_lock<std::mutex> uniq_lk(recvMutex_);
			recvCond_.wait(uniq_lk, [this] { return 1 == recvCond_val; });
			recvCond_val = 0;
			DEBUG_PRINT("recive startt >>>>");

			// 受信内容を取り出す
			std::ofstream  fout;
			fout.open("./recv.jpg", std::ios::out | std::ios::binary | std::ios::trunc);

			std::vector<ResponseParam*>::iterator it = this->response.begin();
			while (it != this->response.end()) {
				auto pResData = *it;
				fout.write(reinterpret_cast<char *>(pResData->resData.buf), pResData->resData.bufsize);

				delete pResData->resData.buf;
				delete pResData;
				it = this->response.erase(it);
			}
			DEBUG_PRINT("recive end <<<");

		}

		DEBUG_PRINT("END");

		return 0;
	}

	void ClientChannel::recvThreadProc() {
		DEBUG_PRINT("Start");

		this->recvSoc = createAcceptSocket(this->recvPortNum);

		// 受信側の準備が完了したことを通知する
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cond_val = 0;
		}
		cond_.notify_one();

		while (true) {
			// 受信データがそろったらクライアントへ通知する
			DEBUG_PRINT("recive loop start >>>>");
			{
				while(true) {
					ResponseParam* pResponseData = new ResponseParam;
					memset(pResponseData, 0, sizeof(ResponseParam));
					std::lock_guard<std::mutex> lock(recvMutex_);

					DEBUG_PRINT("recive start >>>>");


					int n = recv(this->recvSoc, (char*)(pResponseData),
						sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t), 0);

					ResponseData* pResData = &pResponseData->resData;

					DEBUG_PRINT("recive : size[%d] commandType[%d] continueFlg[%d] bufsize[%d]", n,
						(pResponseData->cmdType),
						(pResData->continueFlg),
						(pResData->bufsize));

					pResData->buf = new char[pResData->bufsize];

					int length = 0;
					while(length < pResData->bufsize) {
						DEBUG_PRINT("recive start >>>>");
						int recvSize = pResData->bufsize - length;
						n = recv(this->recvSoc, &pResponseData->resData.buf[length], recvSize, 0);
						length += n;
						DEBUG_PRINT("recive : size[%d]", n);
					}

					this->response.push_back(pResponseData);

					// 残りのデータがないので処理をやめる
					if (pResponseData->resData.continueFlg == 0)
						break;
				}

				recvCond_val = 1;
			}
			recvCond_.notify_one();
			DEBUG_PRINT("recive loop end <<<");
		}

	}

}