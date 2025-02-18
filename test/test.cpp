#include "iocpworker.h"
#include "iocpclient.h"

#include <iostream>

constexpr std::string_view SERVER_IP = "127.0.0.1";
constexpr uint16_t SERVER_PORT = 8888;

int main()
{
    std::cout << "hello" << std::endl;

    {
		IocpWorker worker;
        IocpClient client(SERVER_IP.data(), SERVER_PORT);

        client.SetRequest("Hello, this is iocpClient");
        worker.Post(&client);

        std::cin.get();
		std::cout << "response: " << client.GetResponse() << std::endl;
    }
}