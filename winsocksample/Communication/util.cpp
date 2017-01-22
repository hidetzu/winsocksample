
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
		DEBUG_PRINT("[%s];%d: ret[%d]", __func__, __LINE__, ret);

		return soc;
	}

	SOCKET createAcceptSocket(u_short portNum) {
		DEBUG_PRINT("Start");

		SOCKET sock;
		SOCKET acSock;

		struct sockaddr_in addr;
		struct sockaddr_in client;
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
		DEBUG_PRINT("listen");

		// TCP�N���C�A���g����̐ڑ��v�����󂯕t����
		len = sizeof(client);

		acSock = accept(sock, (struct sockaddr *)&client, &len);
		DEBUG_PRINT("END accept[%d]", (int)acSock);
		return acSock;
	}
};