#pragma once

#include <WinSock2.h>
#include <iostream>
#include <Ws2tcpip.h>
#include <thread>
#include <mutex>
#include <vector>
#include <ctime>
#include <condition_variable>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...)                                                            \
				do {                                                                     \
					fprintf(stdout, "[%s]:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__); \
					fprintf(stdout, "\n");                                               \
				} while (0);
#else

#define DEBUG_PRINT(fmt, ...)                                                            

#endif

