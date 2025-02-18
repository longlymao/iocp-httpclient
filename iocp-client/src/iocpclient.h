/*
 * Copyright (c) 2025 longlymao
 * All rights reserved.
 * MIT License
 */
#pragma once

#include "iocpcontext.h"
#include <string>

class IocpClient {
public:
	IocpClient(const std::string& host, uint16_t port);
	~IocpClient();

	void SetRequest(const std::string& request);
	std::string GetResponse();

	void OnFailed();
	void OnSuccess();

	void AppendResponse(const std::string& response);

	IocpContext& GetContext() { return context; }
	std::string GetHost() { return host; }
	uint16_t GetPort() { return port; }

private:
	IocpContext context;
	std::string host;
	uint16_t port;

	std::string request;
	std::string response;
};