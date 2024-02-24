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
#include <utility>
class Slave{
private:
    SOCKET sock;
    sockaddr_in slave;

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
    Slave(const std::string& slave_ip, int port) {
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

        slave.sin_addr.s_addr = inet_addr(slave_ip.c_str());
        slave.sin_family = AF_INET;
        slave.sin_port = htons(port);

        // Connect to remote slave
        if (connect(sock, (struct sockaddr *)&slave, sizeof(slave)) == SOCKET_ERROR) {
            std::cerr << "Connection failed: " << WSAGetLastError() << "\n";
            closesocket(sock);
            WSACleanup();
            return;
        }

        std::cout << "Connected\n";
    }

    ~Slave() {
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
    Slave slave("127.0.0.1", 12345); // Start slave on port 12345
    int sizeint, threadSizeint, size, start, end, threadNum;
    std::vector<int> primes;
    // Receive message from client
    slave.sendMessage("1");
   

    std::string input = slave.receiveMessage();

    std::cout << "Message from Main: " << input << std::endl;
    std::stringstream ss(input);
    std::tuple<int, int, int> result;
    char delimiter;
    ss >> std::get<0>(result) >> delimiter >> std::get<1>(result) >> delimiter >> std::get<2>(result);

    start = std::get<0>(result);
    end = std::get<1>(result);
    threadNum = std::get<2>(result);
  
    size = end - start + 1;
    // std::string threadSize = slave.receiveMessage();

    std::cout << "size: " << size<< std::endl;
    std::cout << "threadSizeint: " << threadNum<< std::endl;


    std::mutex primesMutex;



    int chunkSize = size / threadNum;
    std::vector<std::thread> threads;
     std::cout << " out loop" << std::endl;
     std::cout << start << std::endl;
    std::cout << end << std::endl;
    for (int i = 0; i < threadNum; ++i) {
        int startRange = (i==0 ? start : i * chunkSize + 1 + start);
        int endRange =  (i ==  threadNum - 1 ? end: (i + 1) * chunkSize + start);
        //std::cout << " in loop" << std::endl;
        threads.emplace_back(&Slave::findPrimes, &slave, startRange, endRange, std::ref(primes), std::ref(primesMutex));
    }
    //std::cout << " done22 loop" << std::endl;
    for (auto& thread : threads) {
        thread.join();
    }
    //std::cout << " done loop" << std::endl;
    slave.sendMessage(std::to_string(primes.size()));
    bool checker = true;
    for(int i = 0; i < primes.size(); i++){
        if (slave.check_prime(primes.at(i)) == false){
            checker = false;
            break;
        };
    }
    if(checker){
        std::cout << "All numbers in list of Slave are validated as prime" << std::endl;
    }
    else{
        std::cout << "List Contains non prime numbers" << std::endl;
    }

   
    std::cout << primes.size() << " primes were found." << std::endl;
  
 
    // sizeint= std::stoi(size); 
    // threadSizeint= std::stoi(threadSize); 
    return 0;
}

