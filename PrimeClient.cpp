#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <utility>
#include <sstream>
#pragma comment(lib, "ws2_32.lib")

class Client {
private:
    SOCKET sock;
    sockaddr_in server;

public:
    Client(const std::string& server_ip, int port) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << "\n";
            return;
        }

        // Create socket
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Could not create socket: " << WSAGetLastError() << "\n";
            WSACleanup();
            return;
        }

        server.sin_addr.s_addr = inet_addr(server_ip.c_str());
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        // Connect to remote server
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
            std::cerr << "Connection failed: " << WSAGetLastError() << "\n";
            closesocket(sock);
            WSACleanup();
            return;
        }

        std::cout << "Connected\n";
    }

    ~Client() {
        closesocket(sock);
        WSACleanup();
    }

    void sendMessage(const std::string& message) {
        if (send(sock, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << "\n";
        }
    }
     std::string receiveMessage() {
        char buffer[256];
        int bytesReceived = recv(sock, buffer, 256, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Receive failed: " << WSAGetLastError() << "\n";
            return "";
        }
        buffer[bytesReceived] = '\0'; // Null-terminate the string
        return std::string(buffer);
    }

};

int main() {
    int size, threads;
    std::pair<int, int> inputPair;
    Client client("127.0.0.1", 12345); // Connect to server running on localhost, port 12345
  
    client.sendMessage("0");
     
     


    std::cout << "Size:";
    std::cin >> size;
    std::cout << "Thread size:";
    std::cin >> threads;
    if(threads <= 0){
        threads = 1;
    }
   
    inputPair.first = size;
    inputPair.second = threads;
    client.sendMessage(std::to_string(inputPair.first) + "," + std::to_string(inputPair.second));
    std::string receivedMessage = client.receiveMessage();
    std::stringstream ss(receivedMessage);
    std::pair<int, int> result;
    char delimiter;
    ss >> result.first >> delimiter >> result.second;
    std::cout << "Received message from server: " << result.first << "  " << result.second << std::endl;
    return 0;
}
