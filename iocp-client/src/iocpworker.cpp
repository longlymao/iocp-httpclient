/*
 * Copyright (c) 2025 longlymao
 * All rights reserved.
 * MIT License
 */
#include "iocpworker.h"

#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "Ws2_32.lib")

static LPFN_CONNECTEX LoadConnectEx() {
	GUID guid = WSAID_CONNECTEX;
	LPFN_CONNECTEX connectEx = nullptr;
	DWORD bytes = 0;
	SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectEx, sizeof(connectEx), &bytes, nullptr, nullptr);
	closesocket(socket);
	return connectEx;
}

static LPFN_CONNECTEX GetConnectEx() {
	static LPFN_CONNECTEX connectEx = LoadConnectEx();
	return connectEx;
}

IocpWorker::IocpWorker(int threadCount) : threadCount(threadCount) {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	CreateIocp();
	CreateWorkThreads();
}

IocpWorker::~IocpWorker() {
	ClearWorkThread();
	ClearIocp();
}

void IocpWorker::CreateIocp() {
	iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (iocp == nullptr) {
		std::cerr << "CreateIoCompletionPort failed with error: " << GetLastError() << std::endl;
	}
}

void IocpWorker::CreateWorkThreads() {
	for (int i = 0; i < threadCount; i++) {
		threads.emplace_back(&IocpWorker::WorkThreadProc, this);
	}
}

void IocpWorker::ClearIocp() {
	if (iocp != nullptr) {
		CloseHandle(iocp);
		iocp = nullptr;
	}
}

void IocpWorker::ClearWorkThread() {
	for (int i = 0; i < threadCount; i++) {
		PostQueuedCompletionStatus(iocp, 0, 0, nullptr);
	}
	for (auto& thread : threads) {
		thread.join();
	}
}

void IocpWorker::WorkThreadProc() {
	DWORD bytesTransferred = 0;
	ULONG_PTR completionKey = 0;
	LPOVERLAPPED overlapped = nullptr;

	while (true) {
		BOOL result = GetQueuedCompletionStatus(iocp, &bytesTransferred, &completionKey, &overlapped, INFINITE);

		if (result && bytesTransferred == 0 && completionKey == 0 && overlapped == nullptr) {
			std::cout << "GetQueuedCompletionStatus returned true with all zeros" << std::endl;
			break;
		}

		if (!result && overlapped == nullptr) {
			std::cerr << "GetQueuedCompletionStatus failed: " << GetLastError() << std::endl;
			break;
		}

		IocpClient* client = reinterpret_cast<IocpClient*>(completionKey);
		if (client == nullptr) {
			std::cerr << "GetQueuedCompletionStatus returned nullptr client" << std::endl;
			break;
		}

		if (&client->GetContext().overlapped != overlapped) {
			std::cerr << "GetQueuedCompletionStatus returned different overlapped" << std::endl;
			break;
		}

		if (client->GetContext().operation == IocpOperation::NONE) {
			if (result) {
				Connect(client);
			}
			else {
				client->OnFailed();
			}
		}
		else if (client->GetContext().operation == IocpOperation::CONNECT) {
			if (!result) {
				client->OnFailed();
			}
			else {
				Send(client);
			}
		}
		else if (client->GetContext().operation == IocpOperation::WRITE) {
			if (!result) {
				client->OnFailed();
			}
			else {
				Read(client);
			}
		}
		else if (client->GetContext().operation == IocpOperation::READ) {
			if (bytesTransferred == 0) {
				client->OnFailed();
			}
			else {
				client->AppendResponse(client->GetContext().buffer);
				client->OnSuccess();
			}
		}

		std::cout << "bytesTransferred: " << bytesTransferred << std::endl;
	}
}

void IocpWorker::Post(IocpClient* client) {
	client->GetContext().socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (client->GetContext().socket == INVALID_SOCKET) {
		std::cerr << "WSASocket failed with error: " << WSAGetLastError() << std::endl;
		return;
	}

	struct sockaddr_in addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = 0;
	if (bind(client->GetContext().socket, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl;
		return;
	}

	if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(client->GetContext().socket), iocp, reinterpret_cast<ULONG_PTR>(client), 0) == nullptr) {
		std::cerr << "CreateIoCompletionPort failed with error: " << GetLastError() << std::endl;
		return;
	}

	client->GetContext().operation = IocpOperation::NONE;
    PostQueuedCompletionStatus(iocp, 0, reinterpret_cast<ULONG_PTR>(client), &client->GetContext().overlapped);
}

void IocpWorker::Connect(IocpClient* client) {
	LPFN_CONNECTEX ConnectEx = GetConnectEx();
	if (ConnectEx == nullptr) {
		std::cerr << "ConnectEx is nullptr" << std::endl;
		return;
	}

	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(client->GetPort());
	inet_pton(AF_INET, client->GetHost().c_str(), &serverAddr.sin_addr);

	client->GetContext().operation = IocpOperation::CONNECT;

	if (!ConnectEx(client->GetContext().socket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr), nullptr, 0, nullptr, &client->GetContext().overlapped)) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			std::cerr << "ConnectEx failed with error: " << WSAGetLastError() << std::endl;
			return;
		}
	}
}

void IocpWorker::Send(IocpClient* client) {
	client->GetContext().operation = IocpOperation::WRITE;
	client->GetContext().wsaBuf.buf = const_cast<char*>(client->GetContext().buffer.c_str());
	client->GetContext().wsaBuf.len = client->GetContext().buffer.size();
	WSASend(client->GetContext().socket, &client->GetContext().wsaBuf, 1, nullptr, 0, &client->GetContext().overlapped, nullptr);
}

void IocpWorker::Read(IocpClient* client) {
	client->GetContext().operation = IocpOperation::READ;
	client->GetContext().buffer.resize(1024, 0);
	client->GetContext().wsaBuf.buf = const_cast<char*>(client->GetContext().buffer.c_str());
	client->GetContext().wsaBuf.len = client->GetContext().buffer.size();
	DWORD flags = 0;
	WSARecv(client->GetContext().socket, &client->GetContext().wsaBuf, 1, nullptr, &flags, &client->GetContext().overlapped, nullptr);
}