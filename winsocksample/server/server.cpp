// winsocksample.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include <stdio.h>
#include "communication.h"

int main() {
	communication_init();

	void* pContext = communication_serverInit();
	communication_serverFinalize(pContext);

	communication_finalize();

	getwchar();
	return 0;
}