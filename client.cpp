#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port>" << std::endl;
        return 1;
    }

    const char* server_ip = argv[1];
    int port = std::stoi(argv[2]);

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Error creating socket" << std::strerror(errno) << std::endl;
        return 1;
    }

    // Server address information
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        close(sock);
        return 1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Error connecting to server: " << std::strerror(errno) << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Connected to server" << std::endl;

    // Send data to server
    std::string message = "Hello from client!";
    if (send(sock, message.c_str(), message.length(), 0) < 0) {
        std::cerr << "Error sending data: " << std::strerror(errno) << std::endl;
        close(sock);
        return 1;
    }
    std::cout << "Sent message: " << message << std::endl;

    // Receive response from server
    char buffer[1024] = {0};
    if (recv(sock, buffer, 1024, 0) < 0) {
         std::cerr << "Error receiving data: " << std::strerror(errno) << std::endl;
        close(sock);
        return 1;
    }
    std::cout << "Received response: " << buffer << std::endl;

    // Close socket
    close(sock);
    std::cout << "Connection closed" << std::endl;

    return 0;
}