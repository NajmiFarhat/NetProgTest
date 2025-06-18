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

int main() {
    initSockets();

    int serverSocket, clientSocket;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Socket failed");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    listen(serverSocket, 1);
    std::cout << "Server listening on port " << PORT << "...\n";

    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientSocket < 0) {
        perror("Accept failed");
        return 1;
    }

    std::cout << "Client connected.\n";

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) break;

        if (strncmp(buffer, "FILE:", 5) == 0) {
            std::ofstream out("received_file.txt", std::ios::binary);
            out.write(buffer + 5, bytes - 5);
            out.close();
            std::cout << "[File Received]\n";
        } else {
            std::cout << "Client: " << buffer << "\n";
        }
    }

#ifdef _WIN32
    closesocket(clientSocket);
    closesocket(serverSocket);
#else
    close(clientSocket);
    close(serverSocket);
#endif

    cleanupSockets();
    return 0;
}
