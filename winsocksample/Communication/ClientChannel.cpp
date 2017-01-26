
#include "communication.h"
#include "common_private.h"
#include "ClientChannel.h"
#include "util.h"

#include <assert.h>

namespace Communication {
	const std::string myLoggerName = "com_server";

	ClientChannel::ClientChannel(int sendPortNum, int recvPortNum, char* ip) {
		my_logger = spdlog::get(myLoggerName);
		this->sendPortNum = sendPortNum;
		this->recvPortNum = recvPortNum;
		this->ip = ip;

		this->sendSoc = createSendSocket(this->sendPortNum, this->ip);
		auto listenSoc = createServerSocket(this->recvPortNum);
		this->recvSoc = createAcceptSocket(listenSoc);

	}

	std::string ClientChannel::LoggerName()
	{
		return myLoggerName;
	}

	ClientChannel::~ClientChannel() {
	}

	int ClientChannel::SendRecv(RequestParam* pRequestParam, ResponseParam** ppResParam) {
		int64_t sendSize = pRequestParam->dataSize + Command_GetRequestParamHeaderSize();
		my_logger->debug("sendSize[{}]", sendSize);
		my_logger->debug("cmdType[{}]",  pRequestParam->cmdType);
		my_logger->debug("bufSize[{}]",  pRequestParam->dataSize);
		int ret = send(this->sendSoc, (const char*)pRequestParam, sendSize, 0);

		// 受信データがそろったらクライアントへ通知する
		my_logger->debug("recive loop start >>>>");

		ResponseParam* pResponseData = new ResponseParam;
		memset(pResponseData, 0, sizeof(ResponseParam));
		my_logger->debug("recive start >>>>");

		int n = recv(this->recvSoc, (char*)(pResponseData),
			Command_GetResponseParamHeaderSize(), 0);

		ResponseData* pResData = &pResponseData->resData;

		my_logger->debug("recive : size[{}] commandType[{}] result[{}] bufsize[{}]", n,
			(pResponseData->cmdType),
			(pResponseData->result),
			(pResData->bufsize));

		pResData->buf = new char[pResData->bufsize];
		recv_allData(this->recvSoc,
			pResponseData->resData.buf, pResData->bufsize);

		*ppResParam = pResponseData;
		my_logger->debug("END");
		my_logger->flush();

		return ret;
	}
}