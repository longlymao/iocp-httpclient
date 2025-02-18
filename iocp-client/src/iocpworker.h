/*
 * Copyright (c) 2025 longlymao
 * All rights reserved.
 * MIT License
 */
#pragma once

#include <vector>
#include <thread>
#include <Windows.h>

#include "iocpclient.h"

class IocpWorker {
    constexpr static int THREAD_COUNT_DEFAULT = 1;

public:
    explicit IocpWorker(int threadCount = THREAD_COUNT_DEFAULT);
    ~IocpWorker();

private:
    void CreateIocp();
    void CreateWorkThreads();

    void ClearIocp();
    void ClearWorkThread();

    void WorkThreadProc();

public:
	void Post(IocpClient* client);

private:
	void Connect(IocpClient* client);
	void Send(IocpClient* client);
	void Read(IocpClient* client);

private:
    HANDLE iocp = nullptr;
    std::vector<std::thread> threads;
    int threadCount;
};