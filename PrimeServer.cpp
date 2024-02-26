#include <iostream>
#include <string>
#include <WinSock2.h>
#include <typeinfo>
#include <sstream>
#include <chrono>
#include <cmath>
#include <thread>
#include <mutex>
#include <vector>
class Server {
private:
    SOCKET server_fd, new_socket;
  
    struct sockaddr_in address;
    int addrlen = sizeof(address);

public:

    bool check_prime(const int &n) {
        for (int i = 2; i * i <= n; i++) {
            if (n % i == 0) {
            return false;
            }
        }
       
        return true;
    }

    void findPrimes(int start, int end, std::vector<int>& primes, std::mutex& mutex) {
        //std::cout << dump << std::endl;
        // std::cout << start << std::endl;
        // std::cout << end << std::endl;

        for (int i = start; i <= end; ++i) {
            //std::cout << i;
            if (check_prime(i)) {
                std::lock_guard<std::mutex> lock(mutex);
                primes.push_back(i);
            }
        }
    }

    Server(int port) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            return;
        }

        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
            std::cerr << "Socket creation failed\n";
            WSACleanup();
            return;
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        // Binding socket to port
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
            std::cerr << "Bind failed\n";
            closesocket(server_fd);
            WSACleanup();
            return;
        }

        // Listening for connections
        if (listen(server_fd, 3) == SOCKET_ERROR) {
            std::cerr << "Listen failed\n";
            closesocket(server_fd);
            WSACleanup();
            return;
        }

        std::cout << "Server listening on port " << port << std::endl;


        std::cout << "Connection accepted\n";
    }

    ~Server() {
        closesocket(server_fd);
        WSACleanup();
    }


     SOCKET acceptConnection() {
            // Accepting incoming connection
            SOCKET new_socket;
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET) {
                std::cerr << "Accept failed\n";
                return INVALID_SOCKET;
            }
            return new_socket;
        }
        std::string receiveMessage(SOCKET socket) {
            char buffer[1024] = {0};
            int valread = recv(socket, buffer, 1024, 0);
            return std::string(buffer);
        }

        void sendMessage(SOCKET socket, const std::string& message) {
            if (send(socket, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
                std::cerr << "Send failed: " << WSAGetLastError() << "\n";
            }
        }
};

int main() {
    Server server(12345); // Start server on port 12345
    int sizeint, threadSizeint;
    std::vector<int> primes;
    SOCKET clientSocket, slaveSocket = INVALID_SOCKET;
    
    // Receive message from client
    

    std::string message;
    char ans;
    SOCKET new_socket = server.acceptConnection();
    message = server.receiveMessage(new_socket);
    std::cout << "Message from client: " << message << std::endl;
    if(message == "0"){
    clientSocket = new_socket;
        std::cout << "new client socket " << std::endl;
    }
    else if(message == "1"){
        slaveSocket = new_socket;
        std::cout << "new slave socket " << std::endl;
    }
    std::cout << "accepting more[y/n]:";
    std::cin >> ans;

    if(ans == 'y'){
        SOCKET new_socket = server.acceptConnection();
        message = server.receiveMessage(new_socket);
        std::cout << "Message from client: " << message << std::endl;
        if(message == "0"){
        clientSocket = new_socket;
            std::cout << "new client socket " << std::endl;
        }
        else if(message == "1"){
            slaveSocket = new_socket;
            std::cout << "new slave socket " << std::endl;
        }
    }


    std::string input = server.receiveMessage(clientSocket);
    int limit;
    std::cout << "Message from client: " << input << std::endl;
    std::stringstream ss(input);
    std::pair<int, int> result;
    char delimiter;
    ss >> result.first >> delimiter >> result.second;
     
    sizeint = result.first;
    threadSizeint = result.second;
 
    
    std::cout << "size: " << sizeint<< std::endl;
    std::cout << "threadSizeint: " << threadSizeint<< std::endl;

    limit = result.first;
  
    std::mutex primesMutex;

    auto start_time = std::chrono::high_resolution_clock::now();
    int split, chunkSize;
    if(slaveSocket != INVALID_SOCKET){
        split = limit / 2;
        server.sendMessage(slaveSocket, std::to_string(split + 1) + "," + std::to_string(limit) + "," + std::to_string(threadSizeint));
        chunkSize = split / threadSizeint;
        limit = split;
    }
    else{
         chunkSize = limit / threadSizeint;
    }
  
    std::vector<std::thread> threads;

    for (int i = 0; i < threadSizeint; ++i) {
        int startRange = (i==0 ? 2 : i * chunkSize + 1);
        int endRange = (i == threadSizeint - 1 ? limit : (i + 1) * chunkSize);
       
        threads.emplace_back(&Server::findPrimes, &server, startRange, endRange, std::ref(primes), std::ref(primesMutex));
    }
   
    for (auto& thread : threads) {
        thread.join();
    }

    int primeCount;
    primeCount = primes.size();
     bool checker = true;
    for(int i = 0; i < primes.size(); i++){
        if (server.check_prime(primes.at(i)) == false){
            checker = false;
            break;
        };
    }
    if(checker){
        std::cout << "All numbers in list of Master are validated as prime" << std::endl;
    }
    else{
        std::cout << "List Contains non prime numbers" << std::endl;
    }

    if(slaveSocket != INVALID_SOCKET){
         std::string primesSlave = server.receiveMessage(slaveSocket);
         std::cout << "Message From Slave:" << primesSlave << std::endl;
         std::stringstream ss(primesSlave);
         std::tuple<int, bool> response; 
         char delimiter;
         ss >> std::get<0>(response) >> delimiter >> std::get<1>(response);
         primeCount += std::get<0>(response);
         if(!std::get<1>(response)){
            checker = false;
         }
         
         
    }
    


    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    if(slaveSocket != INVALID_SOCKET && !checker){
        std::cout << primeCount << "Search Failed. List Contains non-primes" << std::endl;
    }

    std::cout << primeCount << " primes were found." << std::endl;
    std::cout << "Runtime: " << duration.count() << std::endl;
    server.sendMessage(clientSocket,std::to_string(primeCount) + "," + std::to_string(duration.count()));
    // sizeint= std::stoi(size); 
    // threadSizeint= std::stoi(threadSize); 
    return 0;
}

