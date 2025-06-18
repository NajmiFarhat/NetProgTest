#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <string.h>
    #include <errno.h>
#endif

#include <iostream>
#include <fstream>

#define SERVER_IP "10.10.0.2"
#define PORT 8888
#define BUFFER_SIZE 1024

void initSockets() {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

void cleanupSockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void sendFile(int sock) {
    std::ifstream in("file_to_send.txt", std::ios::binary);
    char buffer[BUFFER_SIZE] = "FILE:";
    in.read(buffer + 5, BUFFER_SIZE - 5);
    int bytes = 5 + in.gcount();
    send(sock, buffer, bytes, 0);
    in.close();
    std::cout << "[File Sent]\n";
}

int main() {
    initSockets();

    int clientSocket;
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Socket failed");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connect failed");
        return 1;
    }

    std::cout << "Connected to server.\n";

    std::string message;
    char buffer[BUFFER_SIZE];

    while (true) {
        std::cout << "Enter message (or /file to send a file): ";
        std::getline(std::cin, message);

        if (message == "/file") {
            sendFile(clientSocket);
        } else {
            send(clientSocket, message.c_str(), message.length(), 0);
        }
    }

#ifdef _WIN32
    closesocket(clientSocket);
#else
    close(clientSocket);
#endif

    cleanupSockets();
    return 0;
}
