/*
 * Copyright (c) 2025 longlymao
 * All rights reserved.
 * MIT License
 */
#include "iocpclient.h"
#include <iostream>

IocpClient::IocpClient(const std::string& host, uint16_t port) : host(host), port(port) {
}

IocpClient::~IocpClient() {
}

void IocpClient::SetRequest(const std::string& request) {
	this->request = request;
	context.buffer = request;
}

std::string IocpClient::GetResponse() {
	return response;
}

void IocpClient::OnFailed() {
	std::cerr << "Request failed" << std::endl;
}

void IocpClient::OnSuccess() {
	std::cout << "Request succeeded" << std::endl;
}

void IocpClient::AppendResponse(const std::string& response) {
	this->response += response;
}