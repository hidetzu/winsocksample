// winsocksample.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <thread>
#include <mutex>
#include <condition_variable>

#pragma comment(lib,"Ws2_32.lib")

std::mutex _mtx;
std::condition_variable _cond;

bool _isReady = false;


void recvThread() {
	SOCKET sock;
	char buf[32];

	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len;

	// ソケットの作成
	sock = socket(AF_INET, SOCK_STREAM, 0);

	// 接続先指定用構造体の準備
	addr.sin_family = AF_INET;
	addr.sin_port = htons(15000);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(sock, (struct sockaddr *)&addr, sizeof(addr));


	// TCPクライアントからの接続要求を待てる状態にする
	listen(sock, 5);

	// TCPクライアントからの接続要求を受け付ける
	len = sizeof(client);
	sock = accept(sock, (struct sockaddr *)&client, &len);
	{
		std::lock_guard<std::mutex> lock(_mtx);
		_isReady = true;
	}
	_cond.notify_one();

	{
		std::unique_lock<std::mutex> uniq_lk(_mtx); // ここでロックされる
		_cond.wait(uniq_lk, [] { return !_isReady; });
	}

	while (true) {
		printf("[%s] start\n", __func__);
		// クライアントからデータを受信
		memset(buf, 0, sizeof(buf));
		int n = recv(sock, buf, 5, 0);
		if (n <= 0)
			break;
		printf("[%s] %d, %s\n", __func__ ,n, buf);
		{
			std::lock_guard<std::mutex> lock(_mtx);
			_isReady = true;
		}
		_cond.notify_one();
		printf("[%s]:%d wakeup send\n", __func__, __LINE__);
		{
			std::unique_lock<std::mutex> uniq_lk(_mtx); // ここでロックされる
			_cond.wait(uniq_lk, [] { return !_isReady; });
		}
		printf("[%s]:%d wakeup recv\n", __func__, __LINE__);
		printf("[%s] end\n", __func__);
	}
}

void sendThread() {
	struct sockaddr_in server;
	SOCKET sock;
	char buf[32];

	{
		std::unique_lock<std::mutex> uniq_lk(_mtx); // ここでロックされる
		_cond.wait(uniq_lk, [] { return _isReady; });
	}

	// ソケットの作成
	sock = socket(AF_INET, SOCK_STREAM, 0);

	// 接続先指定用構造体の準備
	server.sin_family = AF_INET;
	server.sin_port = htons(15001);
	server.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	// サーバに接続
	int ret = connect(sock, (struct sockaddr *)&server, sizeof(server));
	if (ret < 0) {
		int nLastErr = WSAGetLastError();
		printf("[%s]:%d ret=%d\n", __func__, __LINE__, nLastErr);
	}

	{
		std::lock_guard<std::mutex> lock(_mtx);
		_isReady = false;
	}
	_cond.notify_one();

	printf("[%s]:%d ret=%d\n", __func__, __LINE__, ret);

	while (true) {
		printf("[%s] send start\n", __func__);
		{
			std::unique_lock<std::mutex> uniq_lk(_mtx); // ここでロックされる
			_cond.wait(uniq_lk, [] { return _isReady; });
		}
		printf("[%s]:%d wakeup recv\n", __func__, __LINE__);
		// 5文字送信
		send(sock, "SELLO", 5, 0);
		{
			std::lock_guard<std::mutex> lock(_mtx);
			_isReady = false;
		}
		_cond.notify_one();
		printf("[%s]:%d wakeup send\n", __func__, __LINE__);
		printf("[%s] end\n", __func__);
	}

	printf("good by!\n");
}


int main()
{
	WSADATA wsaData;

	WSAStartup(MAKEWORD(2, 0), &wsaData);

	std::thread th(recvThread);
	std::thread th2(sendThread);
	th.join();
	th2.join();

	WSACleanup();
    return 0;
}

