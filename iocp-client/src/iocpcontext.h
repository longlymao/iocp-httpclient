/*
 * Copyright (c) 2025 longlymao
 * All rights reserved.
 * MIT License
 */
#pragma once
#include <Windows.h>
#include <WinSock2.h>
#include <string>

enum class IocpOperation {
	NONE,
	ACCEPT,
	CONNECT,
	READ,
	WRITE,
};

struct IocpContext {
	OVERLAPPED overlapped = { 0 };
	IocpOperation operation = IocpOperation::NONE;
	SOCKET socket = INVALID_SOCKET;
	WSABUF wsaBuf = { 0 };
	std::string buffer;
	size_t bytesTransferred = 0;
};