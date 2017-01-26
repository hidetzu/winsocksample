
#include "common_private.h"

namespace Communication {

	SOCKET createSendSocket(u_short portNum, const std::string ip) {
		SOCKET soc;

		struct sockaddr_in server;
		// �\�P�b�g�̍쐬
		soc = socket(AF_INET, SOCK_STREAM, 0);

		// �ڑ���w��p�\���̂̏���
		server.sin_family = AF_INET;
		server.sin_port = htons(portNum);
		inet_pton(AF_INET, (PCSTR)ip.c_str(), &server.sin_addr);

		// �T�[�o�ɐڑ�
		int ret = connect(soc, (struct sockaddr *)&server, sizeof(server));
		DEBUG_PRINT("ret[%d]",ret);

		return soc;
	}

	SOCKET createServerSocket(u_short portNum) {
		SOCKET sock;

		struct sockaddr_in addr;
		int len;

		// �\�P�b�g�̍쐬
		sock = socket(AF_INET, SOCK_STREAM, 0);

		// �ڑ���w��p�\���̂̏���
		addr.sin_family = AF_INET;
		addr.sin_port = htons(portNum);
		addr.sin_addr.S_un.S_addr = INADDR_ANY;

		bind(sock, (struct sockaddr *)&addr, sizeof(addr));
		DEBUG_PRINT("bind");

		// TCP�N���C�A���g����̐ڑ��v����҂Ă��Ԃɂ���
		listen(sock, 5);
		DEBUG_PRINT("listen portNum[%d]", portNum);

		return sock;
	}

	SOCKET createAcceptSocket(SOCKET listenSoc) {
		DEBUG_PRINT("Start");
		SOCKET acSock;

		struct sockaddr_in client;
		int len;

		// TCP�N���C�A���g����̐ڑ��v�����󂯕t����
		len = sizeof(client);
		acSock = accept(listenSoc, (struct sockaddr *)&client, &len);
		if (acSock == -1) {
			DEBUG_PRINT("accept error[%d]", WSAGetLastError());
		}
		DEBUG_PRINT("End accept[%d]", (int)acSock);
		return acSock;
	}

	int recv_allData(SOCKET soc, char* pBuf, int64_t bufSize) {
		int length = 0;
		while(length < bufSize) {
			DEBUG_PRINT("recive start >>>>");
			int recvSize = bufSize - length;
			int n = recv(soc, &pBuf[length], recvSize, 0);
			length += n;
			DEBUG_PRINT("recive : size[%d]", n);
		}

		return 0;
	}

	std::string createDispTimestamp(std::time_t* pTm) {
		std::tm tm;
		localtime_s(&tm, pTm);

		char buffer[32];
		std::strftime(buffer, 32, "%Y%m%d%H%M%S", &tm);

		return std::string(buffer);
	}
};